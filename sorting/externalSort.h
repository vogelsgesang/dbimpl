#ifndef __EXTERNAL_SORT_HPP__
#define __EXTERNAL_SORT_HPP__

#include <cstdint> //uint64_t

namespace dbImpl {

void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize);

}
#endif
