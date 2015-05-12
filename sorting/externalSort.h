#ifndef _EXTERNAL_SORT_HPP_
#define _EXTERNAL_SORT_HPP_

#include <cstdint> //uint64_t

namespace dbImpl {

  //reads uint64_t integers from `fdInput`, sorts them using at most
  //`memSize` bytes of main memory and writes the result to `fdOutput`.
  //`size` gives the number of integers to be sorted.
  void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize);

}
#endif
