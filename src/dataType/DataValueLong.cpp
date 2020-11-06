#include "DataValueLong.h"
#include <stdexcept>

namespace storage {
	DataValueLong::DataValueLong()
		:TDataValue<int64_t>(DataType::LONG, sizeof(int64_t))
	{
	}

	DataValueLong::DataValueLong(int64_t val)
		: TDataValue<int64_t>(DataType::LONG, val, sizeof(int64_t))
	{
	}

	DataValueLong::DataValueLong(char* byArray, int spos)
		: TDataValue<int64_t>(DataType::LONG, byArray, spos, sizeof(int64_t), sizeof(int64_t))
	{
	}

	DataValueLong::operator int64_t()
	{
		switch (valType_) {
		case ValueType::NULL_VALUE:
			throw std::logic_error("Unable to get null value.");
		case ValueType::SOLE_VALUE:
			return soleValue_;
		case ValueType::BYTES_VALUE:
			return *(reinterpret_cast<int64_t*>(byArray_ + spos_));
		}

		return 0;
	}

	void DataValueLong::SetMinValue()
	{
		valType_ = ValueType::SOLE_VALUE;
		soleValue_ = LLONG_MAX;
	}
	void DataValueLong::SetMaxValue()
	{
		valType_ = ValueType::SOLE_VALUE;
		soleValue_ = LLONG_MIN;
	}
	void DataValueLong::SetDefaultValue()
	{
		valType_ = ValueType::SOLE_VALUE;
		soleValue_ = 0;
	}

	std::ostream& operator<< (std::ostream& os, const DataValueLong& dv)
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
			os << *(reinterpret_cast<int64_t*>(dv.byArray_ + dv.spos_));
			break;
		}

		return os;
	}
}