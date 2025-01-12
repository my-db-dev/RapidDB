﻿#pragma once
#include "../utils/BytesFuncs.h"
#include "CachePool.h"
#include <cstdlib>
#include <limits>
#include <list>
#include <map>
#include <new>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace storage {
template <class T> class Mallocator {
public:
  typedef T value_type;

  Mallocator() = default;
  template <class U> constexpr Mallocator(const Mallocator<U> &) noexcept {}
  T *allocate(std::size_t n) {
    return (T *)(CachePool::Apply((uint32_t)(n * sizeof(T))));
  }

  void deallocate(T *p, std::size_t n) noexcept {
    CachePool::Release((Byte *)p, (uint32_t)(n * sizeof(T)));
  }
};

template <class T, class U>
inline bool operator==(const Mallocator<T> &, const Mallocator<U> &) {
  return true;
}
template <class T, class U>
inline bool operator!=(const Mallocator<T> &, const Mallocator<U> &) {
  return false;
}

template <class V> using MVector = std::vector<V, Mallocator<V>>;
template <class V> class MVectorPtr : public MVector<V> {
public:
  ~MVectorPtr() {
    for (auto iter = this->begin(); iter != this->end(); iter++) {
      delete *iter;
    }
  }
};

template <class Key, class T>
using MHashMap = std::unordered_map<Key, T, std::hash<Key>, std::equal_to<Key>,
                                    Mallocator<std::pair<const Key, T>>>;

template <class Key>
using MHashSet = std::unordered_set<Key, std::hash<Key>, std::equal_to<Key>,
                                    Mallocator<Key>>;

template <class Key, class T>
using MTreeMap =
    std::map<Key, T, std::less<Key>, Mallocator<std::pair<const Key, T>>>;

template <class Key>
using MTreeSet = std::set<Key, std::less<Key>, Mallocator<Key>>;

template <class T> using MList = std::list<T, Mallocator<T>>;
template <class T> using MDeque = std::deque<T, Mallocator<T>>;

using MString = basic_string<char, std::char_traits<char>, Mallocator<char>>;

inline MString ToMString(int value) {
  char buf[32];
  std::sprintf(buf, "%d", value);
  return MString(buf);
}
inline MString ToMString(long value) {
  char buf[32];
  std::sprintf(buf, "%ld", value);
  return MString(buf);
}
inline MString ToMString(long long value) {
  char buf[32];
  std::sprintf(buf, "%lld", value);
  return MString(buf);
}
inline MString ToMString(unsigned value) {
  char buf[32];
  std::sprintf(buf, "%u", value);
  return MString(buf);
}
inline MString ToMString(unsigned long value) {
  char buf[32];
  std::sprintf(buf, "%lu", value);
  return MString(buf);
}
inline MString ToMString(unsigned long long value) {
  char buf[32];
  std::sprintf(buf, "%llu", value);
  return MString(buf);
}
inline MString ToMString(float value) {
  char buf[32];
  std::sprintf(buf, "%f", value);
  return MString(buf);
}
inline MString ToMString(double value) {
  char buf[32];
  std::sprintf(buf, "%f", value);
  return MString(buf);
}
inline bool MStringEqualIgnoreCase(const MString &lhs, const MString &rhs) {
  if (lhs.size() != rhs.size())
    return false;
  const char *p1 = lhs.c_str();
  const char *p2 = rhs.c_str();
  for (size_t i = lhs.size(); i > 0; --i, p1++, p2++) {
    if (toupper(*p1) != toupper(*p2))
      return false;
  }

  return true;
}
} // namespace storage

template <> struct std::hash<storage::MString> {
  std::size_t operator()(const storage::MString &str) const noexcept {
    return storage::BytesHash((const Byte *)str.c_str(), str.size());
  }
};

template <> struct std::equal_to<storage::MString> {
  bool operator()(const storage::MString &lhs,
                  const storage::MString &rhs) const noexcept {
    return storage::BytesEqual((Byte *)lhs.c_str(), lhs.size(),
                               (Byte *)rhs.c_str(), rhs.size());
  }
};

template <> struct std::less<storage::MString> {
  bool operator()(const storage::MString &lhs,
                  const storage::MString &rhs) const noexcept {
    return (storage::BytesCompare((Byte *)lhs.c_str(), lhs.size(),
                                  (Byte *)rhs.c_str(), rhs.size()) <= 0);
  }
};