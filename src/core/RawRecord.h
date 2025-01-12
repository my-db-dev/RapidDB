﻿#pragma once
#include "../cache/CachePool.h"
#include "../header.h"
#include "ActionType.h"
#include "IndexType.h"
#include <cstdint>

namespace storage {
class IndexPage;
class IndexTree;

class RawRecord {
public:
  RawRecord(IndexTree *indexTree, IndexPage *parentPage, Byte *bys, bool bSole)
      : _parentPage(parentPage), _bysVal(bys), _indexTree(indexTree),
        _bSole(bSole) {}
  RawRecord(RawRecord &src)
      : _bysVal(src._bysVal), _parentPage(src._parentPage),
        _indexTree(src._indexTree), _bSole(src._bSole),
        _actionType(src._actionType), _gapLock(src._gapLock) {
    src._bysVal = nullptr;
  }
  virtual ~RawRecord() {
    if (_bSole && _bysVal != nullptr)
      CachePool::Release(_bysVal, *((uint16_t *)_bysVal));
  }

  inline Byte *GetBysValue() const { return _bysVal; }
  /**Only the bytes' length in IndexPage, key length + value length without
   * overflow page content*/
  inline uint16_t GetTotalLength() const {
    return _bRemoved ? 0 : *((uint16_t *)_bysVal);
  }
  inline uint16_t GetKeyLength() const {
    return *((uint16_t *)(_bysVal + UI16_LEN));
  }
  inline void SetParentPage(IndexPage *page) { _parentPage = page; }
  inline IndexPage *GetParentPage() const { return _parentPage; }
  inline IndexTree *GetTreeFile() const { return _indexTree; }
  virtual uint16_t GetValueLength() const = 0;
  virtual bool IsSole() const { return _bSole; }
  virtual bool IsTransaction() const { return false; }

public:
  static void *operator new(size_t size) {
    return CachePool::Apply((uint32_t)size);
  }
  static void operator delete(void *ptr, size_t size) {
    CachePool::Release((Byte *)ptr, (uint32_t)size);
  }

protected:
  /** the byte array that save key and value's content */
  Byte *_bysVal;
  /** the parent page included this record */
  IndexPage *_parentPage;
  /**index tree*/
  IndexTree *_indexTree;
  /**If this record' value is saved into solely buffer or into index page*/
  bool _bSole;
  // Below is used in LeafRecord, put here to save space. Only vaild when
  // Transaction is not null
  // ActionType
  ActionType _actionType = ActionType::UNKNOWN;
  // This record marked as delete. only use when it has not multi versions and
  // this record will be removed thoroughly.
  bool _bRemoved = false;
  // Gap lock to previous record. NOT NEXT RECORD is due to page will split by
  // last record, the new record will be insert into ahead of the found record.
  bool _gapLock = false;
  /**How many times this record is referenced, it only can be used when the page
   * has been locked. Only valid for LeafPage*/
  uint16_t _refCount = 1;
};
} // namespace storage
