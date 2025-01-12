﻿#pragma once
#include "../cache/Mallocator.h"
#include <exception>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace storage {
using namespace std;
class ErrorMsg;
extern thread_local unique_ptr<ErrorMsg> _threadErrorMsg;

class ErrorMsg : public exception {
public:
  static void LoadErrorMsg(const string &msgPath);
  static void ClearErrorMsg() { _mapErrorMsg.clear(); }

public:
  ErrorMsg() {}
  ErrorMsg(int id, MVector<MString> &&paras = {}) {
    SetMsg(id, std::move(paras));
  }

  ErrorMsg(ErrorMsg &&msg) noexcept {
    _errId = msg._errId;
    _errMsg = std::move(msg._errMsg);
  }

  ErrorMsg &operator=(ErrorMsg &&msg) noexcept {
    _errId = msg._errId;
    _errMsg = std::move(msg._errMsg);
    return *this;
  }

  inline void SetMsg(int errId, MVector<MString> &&paras = {}) {
    _errId = errId;
    auto iter = _mapErrorMsg.find(errId);
    if (iter == _mapErrorMsg.end()) {
      _errMsg = MString("Failed to find the error id, id=" + ToMString(errId));
    } else {
      _errMsg = iter->second;
      for (int i = 0; i < paras.size(); i++) {
        MString str = "{" + ToMString(i + 1) + "}";
        size_t pos = _errMsg.find(str);
        if (pos != MString::npos)
          _errMsg.replace(pos, 3, paras[i]);
      }
    }
  }

  const char *what() const noexcept { return _errMsg.c_str(); }
  int getErrId() { return _errId; }
  int GetSize() { return UI32_LEN * 2 + (int)_errMsg.size(); }
  void SaveMsg(Byte *buf) {
    *(uint32_t *)buf = _errId;
    buf += UI32_LEN;
    *(uint32_t *)buf = (uint32_t)_errMsg.size();
    buf += UI32_LEN;
    BytesCopy((void *)buf, (void *)_errMsg.data(), _errMsg.size());
  }
  void ReadMsg(Byte *buf) {
    _errId = *(uint32_t *)buf;
    buf += UI32_LEN;
    uint32_t len = *(uint32_t *)buf;
    buf += UI32_LEN;
    _errMsg = string((char *)buf, len);
  }

protected:
  static unordered_map<int, MString> _mapErrorMsg;

protected:
  int _errId = 0;
  MString _errMsg;
};
} // namespace storage
