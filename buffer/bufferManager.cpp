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
    frames.reserve(size);
  }
  
  BufferManager::~BufferManager() {
    frames.clear();
    for (auto segment: segmentFds) {
      close(segment.second);
    }
  }
  
  BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {
    std::unique_lock<std::mutex> globalLock(globalMutex);
    twoQ.access(pageId);
    
    if (frames.count(pageId) == 0) { // page is not in buffer
      if (frames.size() >= size) {  // buffer is already full
        // get next page to evict from twoQ
        uint64_t evictedPage = twoQ.evict();
        auto frameIt = frames.find(evictedPage);
        // before deleting the frame, we need to get an exclusive lock on it
        frameIt->second.lock(true);
        // delete reference from hash table
        // the destructor of the BufferFrame will free the memory
        frames.erase(evictedPage);
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
    //TODO: access to segmentFds should be guarded by a lock...
    //      but this issue will be fixed anyway by writing the disks
    //      back to disk when they are evicted instead of unfixed
    if (isDirty) { // if page is dirty, write it to disk
      int segmentId = frame.pageId << segmentBits;
      int segmentFd = segmentFds.at(segmentId);
      int offset = frame.pageId & offsetBitMask;
      
      #if __unix
      posix_fallocate(segmentFd, offset * PAGE_SIZE, PAGE_SIZE);
      #endif
      dbImpl::checkedPwrite(segmentFd, frame.getData(), PAGE_SIZE, offset*PAGE_SIZE);
    }

    frame.unlock();
  }
  
}
