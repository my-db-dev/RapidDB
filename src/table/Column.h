﻿#pragma once
#include "../cache/Mallocator.h"
#include "../dataType/DataType.h"
#include "../dataType/IDataValue.h"
#include "../header.h"
#include "../utils/BytesConvert.h"
#include "../utils/CharsetConvert.h"
#include <any>

namespace storage {
using namespace std;

class BaseColumn {
public:
  BaseColumn() : _name(), _position(), _dataType() {}
  BaseColumn(string name, int32_t pos, DataType dataType)
      : _name(name), _position(pos), _dataType(dataType) {}
  virtual ~BaseColumn() {}
  const string &GetName() const { return _name; }
  int32_t GetPosition() { return _position; }
  DataType GetDataType() { return _dataType; }

public:
  static void *operator new(size_t size) {
    return CachePool::Apply((uint32_t)size);
  }
  static void operator delete(void *ptr, size_t size) {
    CachePool::Release((Byte *)ptr, (uint32_t)size);
  }

protected:
  string _name;       // The column's name
  int32_t _position;  // The position that start from 0
  DataType _dataType; // Which data type for this column
};

class PhysColumn : public BaseColumn {
public:
  PhysColumn()
      : BaseColumn(), _bNullable(false), _comments(), _maxLength(0),
        _initVal(0), _incStep(1), _charset(), _pDefaultVal(nullptr) {}
  PhysColumn(const string &name, int32_t pos, DataType dataType,
             const string &comments, bool bNullable, int32_t maxLen,
             int64_t initVal, int64_t incStep, Charsets charset,
             IDataValue *defaultVal)
      : BaseColumn(name, pos, dataType), _bNullable(bNullable),
        _comments(comments), _maxLength(maxLen), _initVal(initVal),
        _incStep(incStep), _charset(charset), _pDefaultVal(defaultVal) {}
  ~PhysColumn() { delete _pDefaultVal; }

public:
  uint32_t ReadData(Byte *pBuf);
  uint32_t WriteData(Byte *pBuf);
  bool IsNullable() const { return _bNullable; }
  int32_t GetMaxLength() const { return _maxLength; }
  int64_t GetInitVal() const { return _initVal; }
  int64_t GetIncStep() const { return _incStep; }
  Charsets GetCharset() { return _charset; }
  const IDataValue *GetDefaultVal() { return _pDefaultVal; }
  const string &GetComments() { return _comments; }

protected:
  bool _bNullable;    // Can be null or not for this column
  Charsets _charset;  // The charset for char data type
  int32_t _maxLength; // The max length for variable length data type
  // The initalize value for auto-increment column, the default is 0
  int64_t _initVal;
  int64_t _incStep; // The step between two neighbor value, the default is 1
  string _comments; // Comments for this column
  IDataValue *_pDefaultVal; // The default value if has or null
};

class ResultColumn : public BaseColumn {
public:
  ResultColumn(const string &name, uint32_t pos, DataType dataType,
               string alias, int dataBasicStart, int prevVarCols,
               int colNullPlace)
      : BaseColumn(name, pos, dataType), _alias(alias),
        _dataBasicStart(dataBasicStart), _prevVarCols(prevVarCols),
        _colNullPlace(colNullPlace) {}
  ~ResultColumn() {}

  const string &GetAlias() const { return _alias; }
  const int GetDataBasicStart() const { return _dataBasicStart; }
  const int GetPrevVarCols() const { return _prevVarCols; }
  const int GetColNullPlace() const { return _colNullPlace; }

  /**
   * Return the column's data start position, if is null, return -1
   * @param bys the row data begin bytes.
   * @return -1: if value is null; > 0: the actual start position of column data
   */
  int CalcPosition(Byte *bys) {
    if ((bys[sizeof(int32_t) + _position / BYTE_SIZE] &
         (Byte)(1 << (_position % BYTE_SIZE))) == 0) {
      return -1;
    } else {
      return _dataBasicStart +
             (_prevVarCols <= 0
                  ? 0
                  : UInt32FromBytes(bys + _colNullPlace +
                                    _prevVarCols * sizeof(int32_t)));
    }
  }

  /**
   * Return the data length for this column
   * @param bys the row data begin bytes.
   * @return the actual data length, if null, return 0;
   */
  int GetLength(Byte *bys);

  /**
   * compare the same column for two records
   * @param bys1
   * @param bys2
   * @return
   */
  int CompareTo(Byte *bys1, Byte *bys2);
  /**To judge if a column'svalue in a row is null*/
  bool IsNull(Byte *bys) {
    return bys[_position / BYTE_SIZE] & (1 << (_position % BYTE_SIZE));
  }

protected:
  string _alias; // The ailas of this column
  /** The data start position in this record value byte array.
   * This value does not include the variable columns' length.
   * Row data byte array com
   */
  int _dataBasicStart;
  /**To save how many variable columns(String or blob) before this column*/
  int _prevVarCols;
  /**How many bytes to save which columns are null, 8 columns per byte*/
  int _colNullPlace;
};
} // namespace storage
