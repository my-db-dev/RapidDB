#include "DataValueBlob.h"
#include <stdexcept>
#include "../utils/ErrorMsg.h"
#include "../config/ErrorID.h"

namespace storage {
  DataValueBlob::DataValueBlob(uint32_t maxLength, bool bKey)
    : IDataValue(DataType::BLOB, ValueType::NULL_VALUE, bKey), maxLength_(maxLength)
  { }

  DataValueBlob::DataValueBlob(char* val, uint32_t len, uint32_t maxLength, bool bKey)
    : IDataValue(DataType::BLOB, ValueType::SOLE_VALUE, bKey), soleValue_(val),
      maxLength_(maxLength), soleLength_(len)
  {
    if (soleLength_ >= maxLength_)
    {
      delete[] val;
      throw utils::ErrorMsg(DT_INPUT_OVER_LENGTH, { to_string(maxLength_), to_string(soleLength_) });
    }
  }

  DataValueBlob::DataValueBlob(const char* val, uint32_t len, uint32_t maxLength, bool bKey)
    : IDataValue(DataType::BLOB, ValueType::SOLE_VALUE, bKey),
    maxLength_(maxLength), soleLength_(len)
  {
    if (soleLength_ >= maxLength_)
    {
      throw utils::ErrorMsg(DT_INPUT_OVER_LENGTH, { to_string(maxLength_), to_string(soleLength_) });
    }
     
    soleValue_ = new char[soleLength_];
    memcpy(soleValue_, val, soleLength_);
  }

  DataValueBlob::DataValueBlob(Byte* byArray, uint32_t len, uint32_t maxLength, bool bKey)
    : IDataValue(DataType::BLOB, ValueType::BYTES_VALUE, bKey), byArray_(byArray),
      maxLength_(maxLength), soleLength_(len)
  { }

  DataValueBlob::DataValueBlob(uint32_t maxLength, bool bKey, std::any val)
    : IDataValue(DataType::BLOB, ValueType::SOLE_VALUE, bKey), maxLength_(maxLength)
  {
    throw utils::ErrorMsg(DT_UNSUPPORT_CONVERT, { val.type().name(), "DataValueBlob" });
  }

  DataValueBlob::DataValueBlob(const DataValueBlob& src) : IDataValue(src)
  {
    maxLength_ = src.maxLength_;
    soleLength_ = src.soleLength_;

    switch (valType_)
    {
    case ValueType::SOLE_VALUE:
      soleValue_ = new char[strlen(src.soleValue_) + 1];
      memcpy(soleValue_, src.soleValue_, soleLength_);
      break;
    case ValueType::BYTES_VALUE:
      byArray_ = src.byArray_;
      break;
    case ValueType::NULL_VALUE:
    default:
      break;
    }
  }

  DataValueBlob::~DataValueBlob()
  {
    if (valType_ == ValueType::SOLE_VALUE) {
      delete[] soleValue_;
      valType_ = ValueType::NULL_VALUE;
    }
  }

  std::any DataValueBlob::GetValue() const
  {
    switch (valType_)
    {
    case ValueType::SOLE_VALUE:
      return soleValue_;
    case ValueType::BYTES_VALUE:
      return (char*)byArray_;
    case ValueType::NULL_VALUE:
    default:
      return std::any();
    }
  }

  uint32_t DataValueBlob::WriteData(Byte* buf, bool key)
  {
    assert(valType_ != ValueType::BYTES_VALUE);
    if (key)
    {
      if (valType_ == ValueType::SOLE_VALUE)
      {
        std::memcpy(buf, soleValue_, soleLength_);
        return (uint32_t)soleLength_;
      }
      else
      {
        return 0;
      }
    }
    else
    {
      if (valType_ == ValueType::NULL_VALUE)
      {
        *buf = 0;
        return 1;
      }
      else
      {
        *buf = 1;
        buf++;
        *((uint32_t*)buf) = soleLength_;
        buf += sizeof(uint32_t);
        std::memcpy(buf, soleValue_, soleLength_);
        return (uint32_t)soleLength_ + sizeof(uint32_t) + 1;
      }
    }
  }
  
  uint32_t DataValueBlob::GetPersistenceLength(bool key) const 
  {
    assert(valType_ != ValueType::BYTES_VALUE);
    if (key)
    {
      return soleLength_;
    }
    else
    {
      switch (valType_)
      {
      case ValueType::SOLE_VALUE:
        return soleLength_ + 1 + sizeof(uint32_t);
      case ValueType::BYTES_VALUE:
        return soleLength_ + 1 + sizeof(uint32_t);
      case ValueType::NULL_VALUE:
      default:
        return 1;
      }
    }
  }

  uint32_t DataValueBlob::WriteData(Byte* buf)
  {
    if (bKey_)
    {
      if (valType_ == ValueType::BYTES_VALUE)
      {
        std::memcpy(buf, byArray_, soleLength_);
        return soleLength_;
      }
      else if (valType_ == ValueType::SOLE_VALUE)
      {
        std::memcpy(buf, soleValue_, soleLength_);
        return (uint32_t)soleLength_;
      }
      else
      {
        return 0;
      }
    }
    else
    {
      if (valType_ == ValueType::NULL_VALUE)
      {
        *buf = 0;
        return 1;
      }
      else if (valType_ == ValueType::BYTES_VALUE)
      {
        *buf = 1;
        buf++;
        *((uint32_t*)buf) = soleLength_;
        buf += sizeof(uint32_t);
        std::memcpy(buf, byArray_, soleLength_);
        return soleLength_ + sizeof(uint32_t) + 1;
      }
      else
      {
        *buf = 1;
        buf++;
        *((uint32_t*)buf) = soleLength_;
        buf += sizeof(uint32_t);
        std::memcpy(buf, soleValue_, soleLength_);
        return (uint32_t)soleLength_ + sizeof(uint32_t) + 1;
      }
    }
  }
  uint32_t DataValueBlob::ReadData(Byte* buf, uint32_t len)
  {
    if (bKey_)
    {
      if (len == 0)
      {
        valType_ = ValueType::NULL_VALUE;
        return 0;
      }

      valType_ = ValueType::BYTES_VALUE;
      byArray_ = buf;
      soleLength_ = len;
      return len;
    }
    else
    {
      valType_ = (*buf ? ValueType::BYTES_VALUE : ValueType::NULL_VALUE);
      buf++;

      if (valType_ == ValueType::NULL_VALUE)
        return 1;

      soleLength_ = *((uint32_t*)buf);
      buf += sizeof(uint32_t);
      byArray_ = buf;
      return soleLength_ + sizeof(uint32_t) + 1;
    }
  }

  uint32_t DataValueBlob::GetDataLength() const
  {
    if (!bKey_ && valType_ == ValueType::NULL_VALUE)
      return 0;
    else
      return soleLength_;
  }

  uint32_t DataValueBlob::GetMaxLength() const
  {
    return maxLength_;
  }

  uint32_t DataValueBlob::GetPersistenceLength() const
  {
    if (bKey_)
    {
      return soleLength_;
    }
    else
    {
      switch (valType_)
      {
      case ValueType::SOLE_VALUE:
        return soleLength_ + 1 + sizeof(uint32_t);
      case ValueType::BYTES_VALUE:
        return soleLength_ + 1 + sizeof(uint32_t);
      case ValueType::NULL_VALUE:
      default:
        return 1;
      }
    }
  }
  void DataValueBlob::SetMinValue()
  {
    if (valType_ == ValueType::SOLE_VALUE) delete[] soleValue_;

    valType_ = ValueType::SOLE_VALUE;
    soleValue_ = new char[1]{};
    soleLength_ = 1;
  }

  void DataValueBlob::SetMaxValue()
  {
    if (valType_ == ValueType::SOLE_VALUE) delete[] soleValue_;

    valType_ = ValueType::SOLE_VALUE;
    soleValue_ = new char[] { "\\uff\\uff\\uff"};
    soleLength_ = 4;
  }

  void DataValueBlob::SetDefaultValue()
  {
    if (valType_ == ValueType::SOLE_VALUE) delete[] soleValue_;

    valType_ = ValueType::SOLE_VALUE;
    soleValue_ = new char[1]{ 0 };
    soleLength_ = 1;
  }

  DataValueBlob::operator const char* () const
  {
    switch (valType_)
    {
    case ValueType::NULL_VALUE:
      return nullptr;
    case ValueType::SOLE_VALUE:
      return soleValue_;
    case ValueType::BYTES_VALUE:
      return (char*)byArray_;
    }

    return nullptr;
  }

  DataValueBlob& DataValueBlob::operator=(const char* val)
  {
    uint32_t len = *(uint32_t*)val;
    if (len >= maxLength_)
      throw utils::ErrorMsg(DT_INPUT_OVER_LENGTH, { to_string(maxLength_), to_string(soleLength_) });
    if (valType_ == ValueType::SOLE_VALUE) delete[] soleValue_;

    soleLength_ = len;
    valType_ = ValueType::SOLE_VALUE;
    soleValue_ = new char[soleLength_];
    memcpy(soleValue_, val, soleLength_);
    return *this;
  }

  void DataValueBlob::Put(uint32_t len, char* val)
  {
    if (len >= maxLength_)
      throw utils::ErrorMsg(DT_INPUT_OVER_LENGTH, { to_string(maxLength_), to_string(soleLength_) });
    if (valType_ == ValueType::SOLE_VALUE) delete[] soleValue_;

    soleLength_ = len;
    valType_ = ValueType::SOLE_VALUE;
    soleValue_ = val;
  }

  DataValueBlob& DataValueBlob::operator=(const DataValueBlob& src)
  {
    if (valType_ == ValueType::SOLE_VALUE) delete[] soleValue_;

    dataType_ = src.dataType_;
    valType_ = src.valType_;
    bKey_ = src.bKey_;
    maxLength_ = src.maxLength_;
    soleLength_ = src.soleLength_;

    switch (valType_)
    {
    case ValueType::SOLE_VALUE:
      soleValue_ = new char[soleLength_];
      memcpy(soleValue_, src.soleValue_, soleLength_);
      break;
    case ValueType::BYTES_VALUE:
      byArray_ = src.byArray_;
      break;
    case ValueType::NULL_VALUE:
    default:
      break;
    }

    return *this;
  }

  std::ostream& operator<< (std::ostream& os, const DataValueBlob& dv)
  {
    switch (dv.valType_)
    {
    case ValueType::NULL_VALUE:
      os << "nullptr";
      break;
    case ValueType::SOLE_VALUE:
      os << "size=" << dv.soleLength_ << "\tValType:" << ValueType::SOLE_VALUE;
      break;
    case ValueType::BYTES_VALUE:
      os << "size=" << dv.soleLength_ << "\tValType:" << ValueType::BYTES_VALUE;
      break;
    }

    return os;
  }
}
