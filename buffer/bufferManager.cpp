#include "buffer/bufferManager.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <cstring>
#include <stdexcept>
#include <system_error>
#include <sstream>
#include "buffer/definitions.h"
#include "utils/checkedIO.h"

namespace dbImpl {
  
  BufferManager::BufferManager(uint64_t size) : size(size) {
    //by reserving enough space in the map, we can be sure that no
    //rehash will ever be necessary. Hence, we can store iterators
    //to this unordered_map safely without any iterator being invalidated.
    frames.reserve(size);
    // create segment directory if it doesn't exist
    if (mkdir("segments", S_IRWXU) == -1 && errno != 17) {
      int occurredErrno = errno;
      errno = 0; //reset, so that later calls can succeed
      throw std::system_error(std::error_code(occurredErrno, std::system_category()), "unable to create directory \"segments\"");
    }
    folderFd = open("segments", O_DIRECTORY);
    if (folderFd == -1) {
      int occurredErrno = errno;
      errno = 0; //reset, so that later calls can succeed
      throw std::system_error(std::error_code(occurredErrno, std::system_category()), "unable to open directory \"segments\"");
    }
  }
  
  BufferManager::~BufferManager() {
    std::lock_guard<std::mutex> globalLock(globalMutex);
    //flush all dirty pages
    for(auto& frameEntry: frames) {
      BufferFrame& frame = frameEntry.second;
      //wait until everyone finished accessing this page
      frame.lock(true);
      if(frame.dirty) {
          int segmentId = frame.pageId << segmentBits;
          int segmentFd = segmentFds.at(segmentId);
          int offset = (frame.pageId & offsetBitMask)*PAGE_SIZE;
          dbImpl::checkedPwrite(segmentFd, frame.getData(), PAGE_SIZE, offset);
      }
      //nobody will be able to acquire this lock in the meantime
      //since we are holding the globalLock. We must unlock the frame
      //before destructing it. So, now is a good moment to unlock it.
      frame.unlock();
    }
    //close all files
    for (auto segment : segmentFds) {
      close(segment.second);
    }
  }
  
  BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {
    std::unique_lock<std::mutex> globalLock(globalMutex);
    
    // page is not in buffer and the buffer is to small?
    //
    // a simple `if` is not enough since we need to release the global
    // lock during flushing the evicted page's content to disk and
    // another thread might slip in, select the same page for eviction and
    // occupy the slot which was just freed.
    // On the other hand the other we recheck if the page was already loaded
    // in every iteration since another thread might have loaded it in the
    // mean time.
    while (frames.count(pageId) == 0 && frames.size() >= size) {
      // pages which are currently getting flushed to disk are still
      // in the `frames` map but not in the 2Q anymore. Hence, the
      // 2Q might be empty while the `frames` map is full.
      if(twoQ.empty()) {
        while(twoQ.empty()) {
          twoQAccessed.wait(globalLock);
        }
        continue; //we must recheck the loop condition
      }
      // get page to be evicted from twoQ
      uint64_t evictedPageId = twoQ.evict();
      auto frameIt = frames.find(evictedPageId);
      #ifdef DEBUG
      if(frameIt == frames.end()) {
        throw std::logic_error("trying to evict a page which is not in memory");
      }
      #endif
      BufferFrame* evictedFrame = &frameIt->second;
      // before deleting the frame, we need to get an exclusive lock on it
      evictedFrame->lock(true); //TODO: exception safe locking...
      // do we need to flush it?
      if(!evictedFrame->dirty) {
        // page is not dirty
        evictedFrame->unlock();
        // nobody will be able to acquire this lock in the meantime
        // since we are holding the globalLock. We must unlock the frame
        // before destructing it.
        frames.erase(evictedPageId);
      } else {
        // page IS dirty and needs to be flushed
        int segmentId = evictedFrame->pageId << segmentBits;
        int segmentFd = segmentFds.at(segmentId);
        int offset = (evictedFrame->pageId & offsetBitMask)*PAGE_SIZE;
        //release the global lock, write the page
        globalLock.unlock();
        dbImpl::checkedPwrite(segmentFd, evictedFrame->getData(), PAGE_SIZE, offset);
        //Maybe we actually fail evicting this page (see below), so we
        //better mark it as pristine to avoid flushing it multiple times
        evictedFrame->dirty = false;
        //we can not simply acquire the global lock again, since otherwise
        //we would acquire the locks in the wrong order. Instead, we first must
        //unlock the page and then get the global lock.
        evictedFrame->unlock();
        //No locks are held here. (1)
        globalLock.lock();
        //At point (1) we do not hold any locks. Another thread might have already deleted
        //the frame or might use the frame which we are currently trying to evict during
        //this time. If this happens, we simply try to evict another frame.
        //We MUST retrieve the frame from the map again since another thread might have deleted it already
        frameIt = frames.find(evictedPageId);
        if(frameIt != frames.end()) {
          evictedFrame = &frameIt->second;
          //Only try to lock the frame. If it fails, another thread is already using
          //this frame again.
          if(evictedFrame->tryLock(true)) { //point (2)
            if(!evictedFrame->dirty) {
              //nobody will be able to acquire this lock in the meantime
              //since we are holding the globalLock. We must unlock the frame
              //before destructing it.
              evictedFrame->unlock();
              frames.erase(evictedPageId);
              //we MUST remove the page id from the twoQ again although evict() already
              //removed it from the twoQ. At point (1) another thread might have slipped in and
              //accessed the page without modifying it. If it freed its lock before point (2),
              //this other thread might stay unnoticed. So we better erase this frame from the twoQ again...
              twoQ.erase(evictedPageId);
            } else {
              //frame is dirty again? drop this lock and try again...
              evictedFrame->unlock();
            }
          }
        }
      }
    }

    //we need to recheck if the page is in the buffer since the global lock
    //was abandoned at point (1)
    if (frames.count(pageId) == 0) {
      // insert page into unordered_map
      frames.emplace(std::piecewise_construct,
                     std::forward_as_tuple(pageId),
                     std::forward_as_tuple(pageId));
      BufferFrame& frame = frames.at(pageId);
      // inform twoQ about this access
      // DO NOT inform the twoQ about this access before inserting the BufferFrame
      // into the `frames` map. Otherwise another thread could try to evict the page
      // before it was even added to the `frames` map and this would result in
      // a SIGSEGV.
      twoQ.access(pageId);
      twoQAccessed.notify_one();
      // lock the frame and unlock the global lock while data is being loaded
      //TODO: use an exception safe lock mechanism here...
      frame.lock(true); // while loading we need a write lock
      globalLock.unlock(); // globalLock should not be held during disk I/O

      //get the segment file's file descriptor
      //opened file descriptors are cached for performance reasons
      uint64_t segmentId = pageId >> segmentBits;
      int segmentFd;
      if (segmentFds.count(segmentId) == 1) { // segment file exists
        segmentFd = segmentFds.at(segmentId);
      } else { // segment file does not exist
        segmentFd = openat(folderFd, std::to_string(segmentId).c_str(), O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);
        if (segmentFd == -1) {
          int occurredErrno = errno;
          errno = 0; //reset, so that later calls can succeed
          std::ostringstream msg;
          msg << "unable to open segment " << segmentId;
          throw std::system_error(std::error_code(occurredErrno, std::system_category()), msg.str());
        }
        segmentFds[segmentId] = segmentFd;
      }

      int offset = (frame.pageId & offsetBitMask)*PAGE_SIZE;
      // does this page already exist on the disk?
      struct stat segmentStat;
      if(fstat(segmentFd, &segmentStat) != 0) {
        int occurredErrno = errno;
        errno = 0; //reset, so that later calls can succeed
        std::ostringstream msg;
        msg << "unable to stat segment " << segmentId;
        throw std::system_error(std::error_code(occurredErrno, std::system_category()), msg.str());
      }
      if(segmentStat.st_size > offset) {
        // load page from disk
        dbImpl::checkedPread(segmentFd, frame.getData(), PAGE_SIZE, offset);
      } else {
        // initialize the memory
        std::memset(frame.getData(), 0, PAGE_SIZE);
      }

      // lock the frame with the actually requested lock level
      if(!exclusive) {
        //we need to downgrade the lock
        //TODO: the downgrade mechanism
        //        frame.unlock();
        //        frame.lock(exclusive);
        //      would not be thread safe since another thread might
        //      kick in and evict the page in the meantime.
        //      Nobody (except for performance) gets harmed if we hand out an
        //      exclusive lock instead of a reader lock, so we simply skip the
        //      downgrade
      }
      return frame;
    } else {
      // page is in buffer
      BufferFrame& frame = frames.at(pageId);
      frame.lock(exclusive);
      twoQ.access(pageId);
      twoQAccessed.notify_one();
      return frame;
    }
  }
  
  void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
    if(isDirty) {
      frame.dirty = true;
    }
    frame.unlock();
  }
  
}
