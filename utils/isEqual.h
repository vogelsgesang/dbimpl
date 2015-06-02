#ifndef _IS_EQUAL_H_
#define _IS_EQUAL_H_

namespace dbImpl {

template<typename K, typename Comp>
static bool isEqual(K key1, K key2, const Comp& smaller) {
  return !smaller(key1, key2) && !smaller(key2, key1);
}

}

#endif
