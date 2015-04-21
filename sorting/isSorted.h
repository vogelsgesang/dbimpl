#ifndef __IS_SORTED_HPP__
#define __IS_SORTED_HPP__

#include <cstdint> //uint64_t

namespace dbImpl {

  //reads a file consisting out of uint64_t integers from `fdTest`
  //and checks whether the numbers are sorted in ascending order. 
  //`memSize` gives the bytes of available main memory and
  //`size` is the number of integers.
  // Returns true iff numbers are sorted properly.
  bool isSorted(int fdTest, uint64_t size, uint64_t memSize);

}
#endif
