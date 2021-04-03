﻿#pragma once
#include "../core/IndexPage.h"
#include "../core/IndexTree.h"
#include "../utils/SpinMutex.h"
#include "../utils/ThreadPool.h"
#include <map>
#include <thread>
#include <unordered_map>

namespace storage {
using namespace std;

class PageDividePool {
public:
  static const uint64_t MAX_QUEUE_SIZE;
  static const uint32_t BUFFER_FLUSH_INTEVAL_MS;
  static const int SLEEP_INTEVAL_MS;

public:
  static thread *CreateThread();
  static void AddCachePage(IndexPage *page);
  static void SetThreadStatus(bool bSuspend) { _bSuspend = bSuspend; }
  static size_t GetWaitingPageSize() { return _mapPage.size(); }

protected:
  static map<uint64_t, IndexPage *> _mapPage;
  static unordered_map<uint64_t, IndexPage *> _mapTmp;

  static thread *_treeDivideThread;
  static bool _bSuspend;
  static utils::SpinMutex _spinMutex;
};
} // namespace storage
