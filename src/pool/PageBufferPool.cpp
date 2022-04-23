﻿#include "PageBufferPool.h"
#include "../core/IndexTree.h"
#include "../utils/Log.h"
#include "PageDividePool.h"
#include "StoragePool.h"
#include <shared_mutex>

namespace storage {

void PageBufferPool::AddPage(CachePage *page) {
  unique_lock<SharedSpinMutex> lock(_rwLock);
  _mapCache.insert(pair<uint64_t, CachePage *>(page->HashCode(), page));
}

CachePage *PageBufferPool::GetPage(uint64_t hashId) {
  shared_lock<SharedSpinMutex> lock(_rwLock);
  auto iter = _mapCache.find(hashId);
  if (iter != _mapCache.end()) {
    iter->second->UpdateAccessTime();
    iter->second->IncRefCount();
    return iter->second;
  }

  return nullptr;
}

void PageBufferPool::ClearPool() {
  unique_lock<SharedSpinMutex> lock(_rwLock);
  for (auto iter = _mapCache.begin(); iter != _mapCache.end(); iter++) {
    CachePage *page = iter->second;
    while (!page->Releaseable()) {
      this_thread::sleep_for(std::chrono::microseconds(10));
    }

    page->GetIndexTree()->DecPages();
    page->DecRefCount();
  }
  _mapCache.clear();
}

thread *PageBufferPool::CreateThread() {
  thread *t = new thread([]() {
    while (true) {
      this_thread::sleep_for(std::chrono::milliseconds(1000 * 5));
      if (_bStop) {
        ClearPool();
        break;
      }
      if (_bSuspend)
        continue;
      try {
        PoolManage();
      } catch (...) {
        LOG_ERROR << "Catch unknown exception!";
      }
    }
  });
  return t;
}

void PageBufferPool::PoolManage() {
  int64_t numDel = _mapCache.size() - _maxCacheSize;
  if (numDel <= 0) {
    // LOG_INFO << "MaxPage=" << _maxCacheSize << "\tUsedPage=" <<
    // _mapCache.size()
    //  << "\tPageDividePool=" << PageDividePool::GetWaitingPageSize();
    // LOG_INFO << "\tWaitWriteQueue:" <<
    // StoragePool::GetWaitingWriteTaskCount();
    return;
  }

  if (numDel > _prevDelNum * 2) {
    numDel = _prevDelNum * 2;
    if (numDel > 100000)
      numDel = 100000;
  } else if (numDel < _prevDelNum / 2) {
    numDel = _prevDelNum / 2;
    if (numDel < 500)
      numDel = 500;
  }

  _prevDelNum = numDel;
  static auto cmp = [](CachePage *left, CachePage *right) {
    return left->GetAccessTime() < right->GetAccessTime();
  };
  priority_queue<CachePage *, MVector<CachePage *>::Type, decltype(cmp)> queue(
      cmp);

  int refCountPage = 0;
  int refPageCount = 0;

  for (auto iter = _mapCache.begin(); iter != _mapCache.end(); iter++) {
    CachePage *page = iter->second;
    int num = page->GetRefCount();
    if (num > 0) {
      refCountPage++;
      refPageCount += num;
    } else if (num < 0) {
      LOG_ERROR << "Errr page ref count. Id=" << page->GetPageId()
                << "\trefCount=" << num;
    }

    if (!page->Releaseable()) {
      continue;
    }

    if ((int64_t)queue.size() < numDel) {
      queue.push(page);
    } else if (page->GetAccessTime() < queue.top()->GetAccessTime()) {
      queue.push(page);
      queue.pop();
    }
  }

  int delCount = 0;
  _rwLock.lock();
  while (queue.size() > 0) {
    CachePage *page = queue.top();
    queue.pop();

    if (!page->Releaseable()) {
      continue;
    }

    _mapCache.erase(page->HashCode());
    page->DecRefCount();
    delCount++;
  }
  _rwLock.unlock();

  LOG_INFO << "MaxPage=" << _maxCacheSize << "\tUsedPage=" << _mapCache.size()
           << "\tPageDividePool=" << PageDividePool::GetWaitingPageSize();
  LOG_INFO << "Removed page:" << delCount
           << "\tWaitWriteQueue:" << StoragePool::GetWaitingWriteTaskCount()
           << "\tRefCountPage=" << refCountPage
           << "\tRefPageCount=" << refPageCount;
}
} // namespace storage
