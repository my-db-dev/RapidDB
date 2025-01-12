﻿#include "PageBufferPool.h"
#include "../core/IndexTree.h"
#include "../utils/Log.h"
#include "PageDividePool.h"
#include "StoragePool.h"
#include <forward_list>
#include <shared_mutex>

namespace storage {
SpinMutex PageBufferPool::_spinMutex;
uint64_t PageBufferPool::_maxCacheSize =
    Configure::GetTotalCacheSize() / Configure::GetCachePageSize();
int64_t PageBufferPool::_prevDelNum = 100;
// Initialize _mapCache and add lambad to increase page reference when call find
// method
ConcurrentHashMap<uint64_t, CachePage *, true> PageBufferPool::_mapCache(
    100, PageBufferPool::_maxCacheSize, [](CachePage *page) { page->IncRef(); },
    [](CachePage *page) { page->DecRef(); });
ThreadPool *PageBufferPool::_threadPool;
atomic_bool PageBufferPool::_bInThreadPool{false};

void PageBufferPool::AddPage(CachePage *page) {
  _mapCache.Insert(page->HashCode(), page);
}

CachePage *PageBufferPool::GetPage(uint64_t hashId) {
  CachePage *page = nullptr;
  _mapCache.Find(hashId, page);
  return page;
}

void PageBufferPool::StopPool() {
  for (int i = 0; i < _mapCache.GetGroupCount(); i++) {
    _mapCache.Clear(i);
  }

  _threadPool = nullptr;
}

void PageBufferPool::PoolManage() {
  int64_t numDel = _mapCache.Size() - _maxCacheSize * 4 / 5;

  if (numDel > _prevDelNum * 2) {
    numDel = _prevDelNum * 2;
    if (numDel > 100000)
      numDel = 100000;
  } else if (numDel < _prevDelNum / 2) {
    numDel = _prevDelNum / 2;
    if (numDel < 1000)
      numDel = 1000;
  }

  _prevDelNum = numDel;
  int64_t grDel = numDel / _mapCache.GetGroupCount();
  int delCount = 0;

  static auto cmp = [](CachePage *left, CachePage *right) {
    return left->GetAccessTime() < right->GetAccessTime();
  };

  for (int i = 0; i < _mapCache.GetGroupCount(); i++) {
    priority_queue<CachePage *, MDeque<CachePage *>, decltype(cmp)> queue(cmp);
    forward_list<CachePage *> flist;

    for (auto iter = _mapCache.Begin(i); iter != _mapCache.End(i); iter++) {
      CachePage *page = iter->second;

      if (!page->Releaseable()) {
        continue;
      }

      if (page->GetIndexTree()->IsClosed()) {
        flist.push_front(page);
        continue;
      }

      if ((int64_t)queue.size() < grDel) {
        queue.push(page);
      } else if (page->GetAccessTime() < queue.top()->GetAccessTime()) {
        queue.pop();
        queue.push(page);
      }
    }

    if (flist.empty() && queue.empty())
      continue;

    _mapCache.Lock(i);

    for (CachePage *page : flist) {
      _mapCache.Erase(i, page->HashCode());
      delCount++;
    }

    while (queue.size() > 0) {
      CachePage *page = queue.top();
      queue.pop();

      if (!page->Releaseable()) {
        continue;
      }

      _mapCache.Erase(i, page->HashCode());
      delCount++;
    }
    _mapCache.Unlock(i);
  }

  _bInThreadPool.store(false);
  LOG_INFO << "MaxPage=" << _maxCacheSize << "\tUsedPage=" << _mapCache.Size()
           << "\tRemoved page:" << delCount;
}

void PageBufferPool::AddTimerTask() {
  TimerThread::AddCircleTask("PageBufferPool", 5000000, []() {
    assert(_threadPool != nullptr);
    bool b = _bInThreadPool.exchange(true, memory_order_relaxed);
    if (b)
      return;

    PagePoolTask *task = new PagePoolTask();
    _threadPool->AddTask(task);
  });
}

void PageBufferPool::RemoveTimerTask() {
  TimerThread::RemoveTask("PageBufferPool");
}

void PageBufferPool::PushTask() {
  assert(_threadPool != nullptr);
  bool b = _bInThreadPool.exchange(true, memory_order_relaxed);
  if (b)
    return;

  PagePoolTask *task = new PagePoolTask();
  _threadPool->AddTask(task);
}
} // namespace storage
