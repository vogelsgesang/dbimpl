#ifndef __DOWNGRADABLE_RW_LOCK__
#define __DOWNGRADABLE_RW_LOCK__

#include "pthread.h"

namespace dbImpl {

  //a reader/writer lock which is also downgradable.
  //
  //Unfortunately, normal pthread rwlocks are not downgradable.
  class DowngradableRwLock {
    public:
      DowngradableRwLock();
      virtual ~DowngradableRwLock();

      //locks this lock
      //if the calling frame are already holds this lock,
      //this function may deadlock
      void lock(bool exclusive = true);
      //unlocks this frame
      void unlock();

      //downgrades the currently hold lock to a reader lock.
      //It does not matter if you are currently holding a reader or writer lock.
      void downgradeToReaderLock()
    
    private:
      pthread_rwlock_t actualLock;
  }

}

#endif
