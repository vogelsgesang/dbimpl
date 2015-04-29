
#ifndef __BUFFER_FRAME_H
#define __BUFFER_FRAME_H

#include <cstdint>

namespace dbImpl {
  class BufferFrame {
  public:
    void* getData();

  private:
    void* data;


  };
}

#endif //__BUFFER_FRAME_H
