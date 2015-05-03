#include "buffer/bufferManager.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <cstring>
#include <system_error>
#include <boost/format.hpp>
#include "buffer/definitions.h"
#include "utils/checkedIO.h"

namespace dbImpl {
  
  BufferManager::BufferManager(uint64_t size) : size(size) {
    //by reserving enough space in the map, we can be sure that no
    //rehash will ever be necessary. Hence, we can store iterators
    //to this unordered_map safely without any iterator being invalidated.
    frames.reserve(size);
  }
  
  BufferManager::~BufferManager() {
    //flush all dirty pages
    for(auto& frameEntry: frames) {
      BufferFrame& frame = frameEntry.second;
      if(frame.dirty) {
          int segmentId = frame.pageId << segmentBits;
          int segmentFd = segmentFds.at(segmentId);
          int offset = (frame.pageId & offsetBitMask)*PAGE_SIZE;
          dbImpl::checkedPwrite(segmentFd, frame.getData(), PAGE_SIZE, offset);
      }
    }
    //close all files
    for (auto segment: segmentFds) {
      close(segment.second);
    }
  }
  
  BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {
    std::unique_lock<std::mutex> globalLock(globalMutex);
    twoQ.access(pageId);
    
    if (frames.count(pageId) == 0) { // page is not in buffer
      while (frames.size() >= size) {
        // buffer is already full
        // a simple if is not enough since we need to release the global
        // lock during flushing the evicted pages' content to disk and
        // another thread might slip in and occupy the slot which was just
        // freed.
        //
        // get next page to evict from twoQ
        // TODO: twoQ and own map might get out of sync if evicting fails (see below)
        uint64_t evictedPage = twoQ.evict();
        auto frameIt = frames.find(evictedPage);
        BufferFrame& frame = frameIt->second;
        // before deleting the frame, we need to get an exclusive lock on it
        frame.lock(true); //TODO: exception safe locking...
        // do we need to flush it?
        if(!frame.dirty) {
          // page is not dirty
          frames.erase(evictedPage);
        } else {
          // page IS dirty and needs to be flushed
          int segmentId = frame.pageId << segmentBits;
          int segmentFd = segmentFds.at(segmentId);
          int offset = (frame.pageId & offsetBitMask)*PAGE_SIZE;
          //release the global lock, write the page, acquire the global lock
          globalLock.unlock();
          dbImpl::checkedPwrite(segmentFd, frame.getData(), PAGE_SIZE, offset);
          //Maybe we actually fail evicting this page (see below), so we
          //better mark it as pristine to avoid flushing it multiple times
          frame.dirty = false;
          //we can not simply acquire the global lock again, since otherwise
          //we would acquire the locks in the wrong order. Instead, we first must
          //unlock the page and then get the global lock.
          frame.unlock();
          //No locks are held here. (1)
          globalLock.lock();
          //At point (1) we do not hold any locks. Another thread might use the
          //frame which we are currently trying to evict during this time. If this
          //happens, we simply try to evict another frame.
          //Only try to lock the frame. If it fails, another thread is already using
          //this frame again.
          if(frame.tryLock(true) && !frame.dirty) {
            frames.erase(evictedPage);
          }
        }
      }

      // insert page into unordered_map
      frames.emplace(std::piecewise_construct,
                     std::forward_as_tuple(pageId),
                     std::forward_as_tuple(pageId));
      BufferFrame& frame = frames.at(pageId);
      // lock the frame and unlock the global lock while data is being loaded
      frame.lock(true); // while loading we need a write lock
      //TODO: use an exception safe lock mechanism here...
      globalLock.unlock(); // globalLock should not be held during disk I/O

      //get the segment file's file descriptor
      //opened file descriptors are cached for performance reasons
      uint64_t segmentId = pageId >> segmentBits;
      int segmentFd;
      if (segmentFds.count(segmentId) == 1) { // segment file exists
        segmentFd = segmentFds.at(segmentId);
      } else { // segment file does not exist
        // create segment directory
        if (mkdir("segments", S_IRWXU) == -1 && errno != 17) {
          int occurredErrno = errno;
          errno = 0; //reset, so that later calls can succeed
          throw std::system_error(std::error_code(occurredErrno, std::system_category()), "unable to create directory \"segments\"");
        }
        segmentFd = open(("segments/" + std::to_string(segmentId)).c_str(), O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);
        if (segmentFd == -1) {
          int occurredErrno = errno;
          errno = 0; //reset, so that later calls can succeed
          boost::format fmt("unable to open segment %1%");
          fmt % segmentId;
          throw std::system_error(std::error_code(occurredErrno, std::system_category()), fmt.str());
        }
        segmentFds[segmentId] = segmentFd;
      }

      int offset = (frame.pageId & offsetBitMask)*PAGE_SIZE;
      // does this page already exist on the disk?
      struct stat segmentStat;
      if(fstat(segmentFd, &segmentStat) != 0) {
        int occurredErrno = errno;
        errno = 0; //reset, so that later calls can succeed
        boost::format fmt("unable to stat segment %1%");
        fmt % segmentId;
        throw std::system_error(std::error_code(occurredErrno, std::system_category()), fmt.str());
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
        //TODO: this downgrade mechanism is not really thread safe since
        //      another thread might kick in and evict the page in the meantime
        frame.unlock();
        frame.lock(exclusive);
      }
      return frame;
    } else {
      // page is in buffer
      BufferFrame& frame = frames.at(pageId);
      frame.lock(exclusive);
      globalLock.unlock();
      return frame;
    }
  }
  
  void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
    frame.dirty |= isDirty;
    frame.unlock();
  }
  
}
