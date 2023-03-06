﻿#pragma once
#include "../utils/BytesConvert.h"
#include "IDataValue.h"

namespace storage {
using namespace std;

class DataValueFixChar : public IDataValue {
public:
  DataValueFixChar(uint32_t maxLength = DEFAULT_MAX_FIX_LEN)
      : IDataValue(DataType::FIXCHAR, ValueType::NULL_VALUE, SavePosition::ALL),
        maxLength_(maxLength), bysValue_(nullptr) {}
  DataValueFixChar(const char *val, int len)
      : IDataValue(DataType::FIXCHAR, ValueType::SOLE_VALUE, SavePosition::ALL),
        maxLength_(len + 1), bysValue_(nullptr) {
    bysValue_ = CachePool::Apply(maxLength_);
    BytesCopy(bysValue_, val, len);
    bysValue_[maxLength_ - 1] = 0;
  }

  DataValueFixChar(Byte *byArray, uint32_t maxLength, SavePosition svPos)
      : IDataValue(DataType::FIXCHAR, ValueType::BYTES_VALUE, svPos),
        bysValue_(byArray), maxLength_(maxLength) {}

  DataValueFixChar(const DataValueFixChar &src);
  ~DataValueFixChar();

public:
  DataValueFixChar *Clone(bool incVal = false) override {
    if (incVal) {
      return new DataValueFixChar(*this);
    } else {
      return new DataValueFixChar(maxLength_);
    }
  }

  std::any GetValue() const override {
    switch (valType_) {
    case ValueType::SOLE_VALUE:
    case ValueType::BYTES_VALUE:
      return MString((char *)bysValue_, maxLength_ - 1);
    case ValueType::NULL_VALUE:
    default:
      return std::any();
    }
  }

  uint32_t GetPersistenceLength(SavePosition dtPos) const override {
    if (dtPos == SavePosition::KEY) {
      return maxLength_;
    } else {
      switch (valType_) {
      case ValueType::SOLE_VALUE:
      case ValueType::BYTES_VALUE:
        return maxLength_;
      case ValueType::NULL_VALUE:
      default:
        return 0;
      }
    }
  }
  size_t Hash() const override {
    if (valType_ == ValueType::NULL_VALUE)
      return 0;
    size_t h = 0;
    for (uint32_t i = 0; i < maxLength_; i++) {
      h = (h << 1) ^ bysValue_[i];
    }
    return h;
  }
  bool Equal(const IDataValue &dv) const override {
    if (dataType_ != dv.GetDataType())
      return false;
    return *this == (DataValueFixChar &)dv;
  }
  uint32_t GetDataLength() const override {
    if (valType_ == ValueType::NULL_VALUE)
      return 0;
    else
      return maxLength_;
  }
  void ToString(StrBuff &sb) const override {
    if (valType_ == ValueType::NULL_VALUE) {
      return;
    }

    sb.Cat((char *)bysValue_, maxLength_ - 1);
  }

  operator MString() const {
    switch (valType_) {
    case ValueType::NULL_VALUE:
    default:
      return MString("");
    case ValueType::SOLE_VALUE:
    case ValueType::BYTES_VALUE:
      return MString((char *)bysValue_);
    }
  }

  operator string() const {
    switch (valType_) {
    case ValueType::NULL_VALUE:
    default:
      return string("");
    case ValueType::SOLE_VALUE:
    case ValueType::BYTES_VALUE:
      return string((char *)bysValue_);
    }
  }
  void SetNull() override {
    if (valType_ == ValueType::SOLE_VALUE)
      CachePool::Release(bysValue_, maxLength_);

    valType_ = ValueType::NULL_VALUE;
    bysValue_ = nullptr;
  }
  uint32_t GetMaxLength() const override { return maxLength_; }

  bool SetValue(string val) { return SetValue(val.c_str(), (int)val.size()); }
  bool SetValue(const char *val, int len);
  bool PutValue(std::any val) override;
  bool Copy(const IDataValue &dv, bool bMove = true) override;

  uint32_t WriteData(Byte *buf, SavePosition svPos) const override;
  uint32_t ReadData(Byte *buf, uint32_t len, SavePosition svPos,
                    bool bSole = true) override;
  uint32_t WriteData(Byte *buf) const override;
  uint32_t ReadData(Byte *buf) override;

  void SetMinValue() override;
  void SetMaxValue() override;
  void SetDefaultValue() override;

  DataValueFixChar *operator=(const char *val);
  DataValueFixChar *operator=(const MString val);
  DataValueFixChar *operator=(const string val);
  DataValueFixChar *operator=(const DataValueFixChar &src);

  bool operator>(const DataValueFixChar &dv) const {
    if (valType_ == ValueType::NULL_VALUE) {
      return false;
    }
    if (dv.valType_ == ValueType::NULL_VALUE) {
      return true;
    }

    return BytesCompare(bysValue_, maxLength_, dv.bysValue_, dv.maxLength_) > 0;
  }
  bool operator<(const DataValueFixChar &dv) const { return !(*this >= dv); }
  bool operator>=(const DataValueFixChar &dv) const {
    if (valType_ == ValueType::NULL_VALUE) {
      return dv.valType_ == ValueType::NULL_VALUE;
    }
    if (dv.valType_ == ValueType::NULL_VALUE) {
      return true;
    }

    return BytesCompare(bysValue_, maxLength_, dv.bysValue_, dv.maxLength_) >=
           0;
    ;
  }
  bool operator<=(const DataValueFixChar &dv) const { return !(*this > dv); }
  bool operator==(const DataValueFixChar &dv) const {
    if (valType_ == ValueType::NULL_VALUE) {
      return dv.valType_ == ValueType::NULL_VALUE;
    }
    if (dv.valType_ == ValueType::NULL_VALUE) {
      return false;
    }

    return BytesCompare(bysValue_, maxLength_, dv.bysValue_, dv.maxLength_) ==
           0;
  }
  Byte *GetBuff() const override { return bysValue_; }
  bool operator!=(const DataValueFixChar &dv) const { return !(*this == dv); }
  friend std::ostream &operator<<(std::ostream &os, const DataValueFixChar &dv);

protected:
  uint32_t maxLength_;
  Byte *bysValue_;
};

std::ostream &operator<<(std::ostream &os, const DataValueFixChar &dv);
} // namespace storage
