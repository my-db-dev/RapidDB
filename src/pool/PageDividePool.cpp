#include "PageDividePool.h"
#include "../config/Configure.h"

namespace storage {
  const uint64_t PageDividePool::MAX_QUEUE_SIZE = 
    Configure::GetTotalCacheSize() / Configure::GetCachePageSize();
  const uint32_t PageDividePool::BUFFER_FLUSH_INTEVAL_MS = 1 * 1000;
  const int PageDividePool::SLEEP_INTEVAL_MS = 100;
  
  unordered_map<uint64_t, IndexPage*> PageDividePool::_mapPage;
  queue<IndexPage*> PageDividePool::_queuePage;
	thread* PageDividePool::_treeDivideThread = PageDividePool::CreateThread();
  bool PageDividePool::_bSuspend = false;
  utils::SpinMutex PageDividePool::_spinMutex;

	thread* PageDividePool::CreateThread() {
		thread* t = new thread([]() {
			utils::ThreadPool::_threadName = "PageDividePool";

			while (true) {
				if (_queuePage.size() == 0) {
					this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}

				if (_bSuspend) {
					this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}

				_spinMutex.lock();
				IndexPage* page = _queuePage.front();
				_queuePage.pop();
				
				if (page->GetRecordRefCount() > 0 || (!page->IsOverTime(BUFFER_FLUSH_INTEVAL_MS) && !page->IsOverlength())) {
					_queuePage.push(page);
					_spinMutex.unlock();
					continue;
				}

				bool b = page->WriteTryLock();
				if (!b || page->GetRecordRefCount() > 0) {
					if (b) page->WriteUnlock();

					_queuePage.push(page);
					_spinMutex.unlock();
					continue;
				}

				_mapPage.erase(page->GetPageId());
				_spinMutex.unlock();
				bool bPassed = false;
				if (page->GetTotalDataLength() > page->GetMaxDataLength()) {
					bPassed = page->PageDivide();
				}
				else {
					bPassed = page->SaveRecords();
				}

				page->WriteUnlock();
				if (!bPassed) {
					_spinMutex.lock();
					_queuePage.push(page);
					_mapPage.insert(pair<uint64_t, IndexPage*>(page->GetPageId(), page));
					_spinMutex.unlock();
				}
				else {
					page->DecRefCount();
				}
			}
			});
		return t;
	}

	void PageDividePool::AddCachePage(IndexPage* page) {
		lock_guard<utils::SpinMutex> lock(_spinMutex);
		if (_mapPage.find(page->GetPageId()) != _mapPage.end()) {
			return;
		}

		page->IncRefCount();
		page->SetPageLastUpdateTime();
		_mapPage.insert(pair<uint64_t, IndexPage*>(page->GetPageId(), page));
		_queuePage.push(page);
	}
}