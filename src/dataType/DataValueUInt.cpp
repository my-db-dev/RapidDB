﻿#include "DataValueUInt.h"
#include <stdexcept>
#include <string>
#include "../utils/ErrorMsg.h"
#include "../utils/BytesConvert.h"
#include "../config/ErrorID.h"
#include <cstring>
#include <limits.h>

namespace storage {
DataValueUInt::DataValueUInt(bool bKey)
  :IDataValue(DataType::UINT, ValueType::NULL_VALUE, bKey)
{
}

DataValueUInt::DataValueUInt(uint32_t val, bool bKey)
  : IDataValue(DataType::UINT, ValueType::SOLE_VALUE, bKey), soleValue_(val)
{
}

DataValueUInt::DataValueUInt(Byte* byArray, bool bKey)
  : IDataValue(DataType::UINT, ValueType::BYTES_VALUE, bKey), byArray_(byArray)
{
}

DataValueUInt::DataValueUInt(std::any val, bool bKey)
  : IDataValue(DataType::UINT, ValueType::SOLE_VALUE, bKey)
{
  if (val.type() == typeid(int64_t)) soleValue_ = (uint32_t)std::any_cast<int64_t>(val);
  else if (val.type() == typeid(int32_t)) soleValue_ = std::any_cast<int32_t>(val);
  else if (val.type() == typeid(int16_t)) soleValue_ = std::any_cast<int16_t>(val);
  else if (val.type() == typeid(uint64_t)) soleValue_ = (uint32_t)std::any_cast<uint64_t>(val);
  else if (val.type() == typeid(uint32_t)) soleValue_ = std::any_cast<uint32_t>(val);
  else if (val.type() == typeid(uint16_t)) soleValue_ = std::any_cast<uint16_t>(val);
  else if (val.type() == typeid(int8_t)) soleValue_ = std::any_cast<int8_t>(val);
  else if (val.type() == typeid(uint8_t)) soleValue_ = std::any_cast<uint8_t>(val);
  else if (val.type() == typeid(std::string)) soleValue_ = std::stoi(std::any_cast<std::string>(val));
  else throw utils::ErrorMsg(DT_UNSUPPORT_CONVERT, { val.type().name(), "int64_t" });
}

DataValueUInt::DataValueUInt(const DataValueUInt& src)
  : IDataValue(src)
{
  switch (src.valType_)
  {
  case ValueType::SOLE_VALUE:
    soleValue_ = src.soleValue_;
    break;
  case ValueType::BYTES_VALUE:
    byArray_ = src.byArray_;
    break;
  case ValueType::NULL_VALUE:
    break;
  }
}

DataValueUInt* DataValueUInt::CloneDataValue(bool incVal)
{
  return new DataValueUInt(*this);
}

std::any DataValueUInt::GetValue() const
{
  switch (valType_)
  {
  case ValueType::SOLE_VALUE:
    return soleValue_;
  case ValueType::BYTES_VALUE:
    return utils::UInt32FromBytes(byArray_, bKey_);
  case ValueType::NULL_VALUE:
  default:
    return std::any();
  }
}

uint32_t DataValueUInt::WriteData(Byte* buf)
{
  return WriteData(buf, bKey_);
}

uint32_t DataValueUInt::GetPersistenceLength(bool key) const
{
  return key ? sizeof(uint32_t) : (valType_ == ValueType::NULL_VALUE ? 1 : 1 + sizeof(uint32_t));
}

uint32_t DataValueUInt::WriteData(Byte* buf, bool key)
{
  if (key) {
    assert(valType_ != ValueType::NULL_VALUE);
    if (valType_ == ValueType::BYTES_VALUE) {
      std::memcpy(buf, byArray_, sizeof(uint32_t));
    } else if (valType_ == ValueType::SOLE_VALUE) {
      utils::UInt32ToBytes(soleValue_, buf, bKey_);
    }

    return sizeof(uint32_t);
  } else {
    if (valType_ == ValueType::NULL_VALUE) {
      *buf = (Byte)DataType::UINT & DATE_TYPE;
      return 1;
    } else if (valType_ == ValueType::BYTES_VALUE) {
      *buf = VALUE_TYPE | ((Byte)DataType::UINT & DATE_TYPE);
      buf++;
      std::memcpy(buf, byArray_, sizeof(uint32_t));
      return sizeof(uint32_t) + 1;
    } else {
      *buf = VALUE_TYPE | ((Byte)DataType::UINT & DATE_TYPE);
      buf++;
      utils::UInt32ToBytes(soleValue_, buf, bKey_);
      return sizeof(uint32_t) + 1;
    }
  }
}

uint32_t DataValueUInt::ReadData(Byte* buf, uint32_t len)
{
  if (bKey_) {
    valType_ = ValueType::SOLE_VALUE;
    soleValue_ = utils::UInt32FromBytes(buf, bKey_);
    return sizeof(uint32_t);
  } else {
    valType_ = (*buf & VALUE_TYPE ? ValueType::SOLE_VALUE : ValueType::NULL_VALUE);
    buf++;

    if (valType_ == ValueType::NULL_VALUE)
      return 1;

    soleValue_ = utils::UInt32FromBytes(buf, bKey_);
    return sizeof(uint32_t) + 1;
  }
}

uint32_t DataValueUInt::GetDataLength() const
{
  return bKey_ ? sizeof(uint32_t) : (valType_ == ValueType::NULL_VALUE ? 0 : sizeof(uint32_t));
}

uint32_t DataValueUInt::GetMaxLength() const
{
  return sizeof(uint32_t);
}

uint32_t DataValueUInt::GetPersistenceLength() const
{
  return bKey_ ? sizeof(uint32_t) : (valType_ == ValueType::NULL_VALUE ? 1 : 1 + sizeof(uint32_t));
}

void DataValueUInt::SetMinValue()
{
  valType_ = ValueType::SOLE_VALUE;
  soleValue_ = 0;
}
void DataValueUInt::SetMaxValue()
{
  valType_ = ValueType::SOLE_VALUE;
  soleValue_ = UINT_MAX;
}
void DataValueUInt::SetDefaultValue()
{
  valType_ = ValueType::SOLE_VALUE;
  soleValue_ = 0;
}

DataValueUInt::operator uint32_t() const
{
  switch (valType_)
  {
  case ValueType::NULL_VALUE:
    return 0;
  case ValueType::SOLE_VALUE:
    return soleValue_;
  case ValueType::BYTES_VALUE:
    return utils::UInt32FromBytes(byArray_, bKey_);
  }

  return 0;
}

DataValueUInt& DataValueUInt::operator=(uint32_t val)
{
  valType_ = ValueType::SOLE_VALUE;
  soleValue_ = val;
  return *this;
}

DataValueUInt& DataValueUInt::operator=(const DataValueUInt& src)
{
  dataType_ = src.dataType_;
  valType_ = src.valType_;
  bKey_ = src.bKey_;
  switch (src.valType_)
  {
  case ValueType::SOLE_VALUE:
    soleValue_ = src.soleValue_;
    break;
  case ValueType::BYTES_VALUE:
    byArray_ = src.byArray_;
    break;
  case ValueType::NULL_VALUE:
    break;
  }

  return *this;
}

bool DataValueUInt::operator > (const DataValueUInt& dv) const
{
  if (valType_ == ValueType::NULL_VALUE) { return false; }
  if (dv.valType_ == ValueType::NULL_VALUE) { return true; }

  uint32_t v1 = (valType_ == ValueType::SOLE_VALUE ? soleValue_ : utils::UInt32FromBytes(byArray_, bKey_));
  uint32_t v2 = (dv.valType_ == ValueType::SOLE_VALUE ? dv.soleValue_ : utils::UInt32FromBytes(dv.byArray_, dv.bKey_));
  return v1 > v2;
}

bool DataValueUInt::operator < (const DataValueUInt& dv) const
{
  return !(*this >= dv);
}

bool DataValueUInt::operator >= (const DataValueUInt& dv) const
{
  if (valType_ == ValueType::NULL_VALUE) { return dv.valType_ == ValueType::NULL_VALUE; }
  if (dv.valType_ == ValueType::NULL_VALUE) { return true; }

  uint32_t v1 = (valType_ == ValueType::SOLE_VALUE ? soleValue_ : utils::UInt32FromBytes(byArray_, bKey_));
  uint32_t v2 = (dv.valType_ == ValueType::SOLE_VALUE ? dv.soleValue_ : utils::UInt32FromBytes(dv.byArray_, dv.bKey_));
  return v1 >= v2;
}

bool DataValueUInt::operator <= (const DataValueUInt& dv) const
{
  return !(*this > dv);
}

bool DataValueUInt::operator == (const DataValueUInt& dv) const
{
  if (valType_ == ValueType::NULL_VALUE) { return dv.valType_ == ValueType::NULL_VALUE; }
  if (dv.valType_ == ValueType::NULL_VALUE) { return false; }

  uint32_t v1 = (valType_ == ValueType::SOLE_VALUE ? soleValue_ : utils::UInt32FromBytes(byArray_, bKey_));
  uint32_t v2 = (dv.valType_ == ValueType::SOLE_VALUE ? dv.soleValue_ : utils::UInt32FromBytes(dv.byArray_, dv.bKey_));
  return v1 == v2;
}

bool DataValueUInt::operator != (const DataValueUInt& dv) const
{
  return !(*this == dv);
}

std::ostream& operator<< (std::ostream& os, const DataValueUInt& dv)
{
  switch (dv.valType_)
  {
  case ValueType::NULL_VALUE:
    os << "nullptr";
    break;
  case ValueType::SOLE_VALUE:
    os << dv.soleValue_;
    break;
  case ValueType::BYTES_VALUE:
    os << utils::UInt32FromBytes(dv.byArray_, dv.bKey_);
    break;
  }

  return os;
}
}
