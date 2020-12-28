#include "DataValueUShort.h"
#include <stdexcept>
#include <string>
#include "../utils/ErrorMsg.h"
#include "../utils/BytesConvert.h"

namespace storage {
	DataValueUShort::DataValueUShort(bool bKey)
		:IDataValue(DataType::USHORT, ValueType::NULL_VALUE, bKey)
	{
	}

	DataValueUShort::DataValueUShort(uint16_t val, bool bKey)
		: IDataValue(DataType::USHORT, ValueType::SOLE_VALUE, bKey), soleValue_(val)
	{
	}

	DataValueUShort::DataValueUShort(Byte* byArray, bool bKey)
		: IDataValue(DataType::USHORT, ValueType::BYTES_VALUE, bKey), byArray_(byArray)
	{
	}

	DataValueUShort::DataValueUShort(std::any val, bool bKey)
		: IDataValue(DataType::USHORT, ValueType::SOLE_VALUE, bKey)
	{
		if (val.type() == typeid(int64_t)) soleValue_ = (uint16_t)std::any_cast<int64_t>(val);
		else if (val.type() == typeid(int32_t)) soleValue_ = (uint16_t)std::any_cast<int32_t>(val);
		else if (val.type() == typeid(int16_t)) soleValue_ = std::any_cast<int16_t>(val);
		else if (val.type() == typeid(uint64_t)) soleValue_ = (uint16_t)std::any_cast<uint64_t>(val);
		else if (val.type() == typeid(uint32_t)) soleValue_ = (uint16_t)std::any_cast<uint32_t>(val);
		else if (val.type() == typeid(uint16_t)) soleValue_ = std::any_cast<uint16_t>(val);
		else if (val.type() == typeid(int8_t)) soleValue_ = std::any_cast<int8_t>(val);
		else if (val.type() == typeid(uint8_t)) soleValue_ = std::any_cast<uint8_t>(val);
		else if (val.type() == typeid(std::string)) soleValue_ = (uint16_t)std::stoi(std::any_cast<std::string>(val));
		else throw utils::ErrorMsg(2001, { val.type().name(), "DataValueUShort" });
	}

	DataValueUShort::DataValueUShort(const DataValueUShort& src)
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

	std::any DataValueUShort::GetValue() const
	{
		switch (valType_)
		{
		case ValueType::SOLE_VALUE:
			return soleValue_;
		case ValueType::BYTES_VALUE:
			return utils::UInt16FromBytes(byArray_, bKey_);
		case ValueType::NULL_VALUE:
		default:
			return std::any();
		}
	}

	uint32_t DataValueUShort::WriteData(Byte* buf)
	{
		if (bKey_)
		{
			if (valType_ == ValueType::NULL_VALUE)
			{
				std::memset(buf, 0, sizeof(uint16_t));
			}
			else if (valType_ == ValueType::BYTES_VALUE)
			{
				std::memcpy(buf, byArray_, sizeof(uint16_t));
			}
			else if (valType_ == ValueType::SOLE_VALUE)
			{
				utils::UInt16ToBytes(soleValue_, buf, bKey_);
			}

			return sizeof(uint16_t);
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
				std::memcpy(buf, byArray_, sizeof(uint16_t));
				return sizeof(uint16_t) + 1;
			}
			else
			{
				*buf = 1;
				buf++;
				utils::UInt16ToBytes(soleValue_, buf, bKey_);
				return sizeof(uint16_t) + 1;
			}
		}
	}

	uint32_t DataValueUShort::ReadData(Byte* buf, uint32_t len)
	{
		if (bKey_)
		{
			if (len == -1)
			{
				valType_ = ValueType::NULL_VALUE;
				return sizeof(uint16_t);
			}

			valType_ = ValueType::BYTES_VALUE;
			byArray_ = buf;
			return sizeof(uint16_t);
		}
		else
		{
			valType_ = (*buf ? ValueType::BYTES_VALUE : ValueType::NULL_VALUE);
			buf++;

			if (valType_ == ValueType::NULL_VALUE)
				return 1;

			byArray_ = buf;
			return sizeof(uint16_t) + 1;
		}
	}

	uint32_t DataValueUShort::GetDataLength() const
	{
		return bKey_ ? sizeof(uint16_t) : (valType_ == ValueType::NULL_VALUE ? 0 : sizeof(uint16_t));
	}

	uint32_t DataValueUShort::GetMaxLength() const
	{
		return sizeof(uint16_t) + (bKey_ ? 0 : 1);
	}
	
	uint32_t DataValueUShort::GetPersistenceLength() const
	{
		return bKey_ ? sizeof(uint16_t) : (valType_ == ValueType::NULL_VALUE ? 1 : 1 + sizeof(uint16_t));
	}

	void DataValueUShort::SetMinValue()
	{
		valType_ = ValueType::SOLE_VALUE;
		soleValue_ = 0;
	}
	void DataValueUShort::SetMaxValue()
	{
		valType_ = ValueType::SOLE_VALUE;
		soleValue_ = USHRT_MAX;
	}
	void DataValueUShort::SetDefaultValue()
	{
		valType_ = ValueType::SOLE_VALUE;
		soleValue_ = 0;
	}

	DataValueUShort::operator uint16_t() const
	{
		switch (valType_) 
		{
		case ValueType::NULL_VALUE:
			return 0;
		case ValueType::SOLE_VALUE:
			return soleValue_;
		case ValueType::BYTES_VALUE:
			return utils::UInt16FromBytes(byArray_, bKey_);
		}

		return 0;
	}

	DataValueUShort& DataValueUShort::operator=(uint16_t val)
	{
		valType_ = ValueType::SOLE_VALUE;
		soleValue_ = val;
		return *this;
	}

	DataValueUShort& DataValueUShort::operator=(const DataValueUShort& src)
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

	bool DataValueUShort::operator > (const DataValueUShort& dv) const
	{
		if (valType_ == ValueType::NULL_VALUE) { return false; }
		if (dv.valType_ == ValueType::NULL_VALUE) { return true; }

		uint16_t v1 = (valType_ == ValueType::SOLE_VALUE ? soleValue_ : utils::UInt16FromBytes(byArray_, bKey_));
		uint16_t v2 = (dv.valType_ == ValueType::SOLE_VALUE ? dv.soleValue_ : utils::UInt16FromBytes(dv.byArray_, dv.bKey_));
		return v1 > v2;
	}

	bool DataValueUShort::operator < (const DataValueUShort& dv) const
	{
		return !(*this >= dv);
	}

	bool DataValueUShort::operator >= (const DataValueUShort& dv) const
	{
		if (valType_ == ValueType::NULL_VALUE) { return dv.valType_ == ValueType::NULL_VALUE; }
		if (dv.valType_ == ValueType::NULL_VALUE) { return true; }

		uint16_t v1 = (valType_ == ValueType::SOLE_VALUE ? soleValue_ : utils::UInt16FromBytes(byArray_, bKey_));
		uint16_t v2 = (dv.valType_ == ValueType::SOLE_VALUE ? dv.soleValue_ : utils::UInt16FromBytes(dv.byArray_, dv.bKey_));
		return v1 >= v2;
	}

	bool DataValueUShort::operator <= (const DataValueUShort& dv) const
	{
		return !(*this > dv);
	}

	bool DataValueUShort::operator == (const DataValueUShort& dv) const
	{
		if (valType_ == ValueType::NULL_VALUE) { return dv.valType_ == ValueType::NULL_VALUE; }
		if (dv.valType_ == ValueType::NULL_VALUE) { return false; }

		uint16_t v1 = (valType_ == ValueType::SOLE_VALUE ? soleValue_ : utils::UInt16FromBytes(byArray_, bKey_));
		uint16_t v2 = (dv.valType_ == ValueType::SOLE_VALUE ? dv.soleValue_ : utils::UInt16FromBytes(dv.byArray_, dv.bKey_));
		return v1 == v2;
	}

	bool DataValueUShort::operator != (const DataValueUShort& dv) const
	{
		return !(*this == dv);
	}

	std::ostream& operator<< (std::ostream& os, const DataValueUShort& dv)
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
			os << utils::UInt16FromBytes(dv.byArray_, dv.bKey_);
			break;
		}

		return os;
	}
}