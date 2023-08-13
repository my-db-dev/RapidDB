﻿#pragma once
#include "../cache/StrBuff.h"
#include "../config/ErrorID.h"
#include "../dataType/DataValueDigit.h"
#include "../dataType/DataValueFixChar.h"
#include "../dataType/DataValueVarChar.h"
#include "../dataType/IDataValue.h"
#include "../utils/ErrorMsg.h"
#include "BaseExpr.h"
#include <unordered_set>

using namespace std;
namespace storage {
/**
 * @brief save const value for expression
 */
class ExprConst : public ExprData {
public:
  ExprConst(IDataValue *val) : _val(val) { _val->SetReuse(true); }
  ExprConst(int64_t ival) {
    _val = new DataValueLong(ival);
    _val->SetReuse(true);
  }
  ExprConst(double dval) {
    _val = new DataValueDouble(dval);
    _val->SetReuse(true);
  }
  ExprConst(char *sval) {
    _val = new DataValueVarChar(sval, strlen(sval));
    _val->SetReuse(true);
  }
  ExprConst(bool bval) {
    _val = new DataValueBool(bval);
    _val->SetReuse(true);
  }
  ExprConst() { _val = nullptr; }
  ~ExprConst() { delete _val; }
  ExprType GetType() { return ExprType::EXPR_CONST; }
  IDataValue *Calc(VectorDataValue &vdLeft, VectorDataValue &vdRight) override {
    return _val;
  }
  IDataValue *GetValue() { return _val; }
  bool IsNull() { return _val == nullptr; }

public:
  IDataValue *_val;
};

/**
 * @brief return a column value in the destion table for insert or update.
 */
class ExprField : public ExprData {
public:
  ExprField(string &tName, string &name)
      : _tName(move(tName)), _name(move(name)) {}

  ExprType GetType() { return ExprType::EXPR_FIELD; }
  void Preprocess(int rowPos) { _rowPos = rowPos; }

  IDataValue *Calc(VectorDataValue &vdPara, VectorDataValue &vdRow) override {
    return vdRow[_rowPos];
  }

public:
  string _tName; // The table name this field belong to
  string _name;  // The field name (The related column)
  int _rowPos;   // The position in the table.
};

/*
 * @brief Get Parameter from inputed parameter array.
 */
class ExprParameter : public ExprData {
public:
  ExprParameter(int paraPos) : _paraPos(paraPos) {}

  ExprType GetType() { return ExprType::EXPR_PARAMETER; }
  IDataValue *Calc(VectorDataValue &vdLeft, VectorDataValue &vdRight) override {
    return vdLeft[_paraPos];
  }

public:
  int _paraPos; // The position in parameter array.
};

class ExprAdd : public ExprData {
public:
  ExprAdd(ExprData *left, ExprData *right)
      : _exprLeft(left), _exprRight(right) {}
  ~ExprAdd() {
    delete _exprLeft;
    delete _exprRight;
  }

  ExprType GetType() { return ExprType::EXPR_ADD; }
  IDataValue *Calc(VectorDataValue &vdLeft, VectorDataValue &vdRight) override {
    IDataValue *left = _exprLeft->Calc(vdLeft, vdRight);
    IDataValue *right = _exprRight->Calc(vdLeft, vdRight);
    IDataValue *rt = nullptr;

    if (left->IsStringType() || right->IsStringType()) {
      StrBuff sb(left->GetDataLength() * 2 + right->GetDataLength() * 2);
      left->ToString(sb);
      right->ToString(sb);
      rt = new DataValueVarChar(sb.GetBuff(), sb.GetBufLen() + 1);
    } else if (left->IsAutoPrimaryKey() && right->IsAutoPrimaryKey()) {
      rt = new DataValueLong(left->GetLong() + right->GetLong());
    } else if (left->IsDigital() && right->IsDigital()) {
      rt = new DataValueDouble(left->GetDouble() + right->GetDouble());
    } else {
      rt = left->Clone();
    }
    if (!left->IsReuse())
      delete left;
    if (!right->IsReuse())
      delete right;
    return rt;
  }

public:
  ExprData *_exprLeft;
  ExprData *_exprRight;
};

class ExprSub : public ExprData {
public:
  ExprSub(ExprData *left, ExprData *right)
      : _exprLeft(left), _exprRight(right) {}
  ~ExprSub() {
    delete _exprLeft;
    delete _exprRight;
  }

  ExprType GetType() { return ExprType::EXPR_SUB; }
  IDataValue *Calc(VectorDataValue &vdLeft, VectorDataValue &vdRight) override {
    IDataValue *left = _exprLeft->Calc(vdLeft, vdRight);
    IDataValue *right = _exprRight->Calc(vdLeft, vdRight);
    IDataValue *rt = nullptr;

    if (left->IsAutoPrimaryKey() && right->IsAutoPrimaryKey()) {
      rt = new DataValueLong(left->GetLong() - right->GetLong());
    } else if (left->IsDigital() && right->IsDigital()) {
      rt = new DataValueDouble(left->GetDouble() - right->GetDouble());
    } else {
      rt = left->Clone();
    }

    if (!left->IsReuse())
      delete left;
    if (!right->IsReuse())
      delete right;
    return rt;
  }

public:
  ExprData *_exprLeft;
  ExprData *_exprRight;
};

class ExprMul : public ExprData {
public:
  ExprMul(ExprData *left, ExprData *right)
      : _exprLeft(left), _exprRight(right) {}
  ~ExprMul() {
    delete _exprLeft;
    delete _exprRight;
  }

  ExprType GetType() { return ExprType::EXPR_MUL; }
  IDataValue *Calc(VectorDataValue &vdLeft, VectorDataValue &vdRight) override {
    IDataValue *left = _exprLeft->Calc(vdLeft, vdRight);
    IDataValue *right = _exprRight->Calc(vdLeft, vdRight);
    IDataValue *rt = nullptr;

    if (left->IsAutoPrimaryKey() && right->IsAutoPrimaryKey()) {
      rt = new DataValueLong(left->GetLong() * right->GetLong());
    } else if (left->IsDigital() && right->IsDigital()) {
      rt = new DataValueDouble(left->GetDouble() * right->GetDouble());
    } else {
      rt = left->Clone();
    }

    if (!left->IsReuse())
      delete left;
    if (!right->IsReuse())
      delete right;
    return rt;
  }

public:
  ExprData *_exprLeft;
  ExprData *_exprRight;
};

class ExprDiv : public ExprData {
public:
  ExprDiv(ExprData *left, ExprData *right)
      : _exprLeft(left), _exprRight(right) {}
  ~ExprDiv() {
    delete _exprLeft;
    delete _exprRight;
  }

  ExprType GetType() { return ExprType::EXPR_DIV; }
  IDataValue *Calc(VectorDataValue &vdLeft, VectorDataValue &vdRight) override {
    IDataValue *left = _exprLeft->Calc(vdLeft, vdRight);
    IDataValue *right = _exprRight->Calc(vdLeft, vdRight);
    IDataValue *rt = nullptr;

    if (left->IsAutoPrimaryKey() && right->IsAutoPrimaryKey()) {
      int64_t r = right->GetLong();
      if (r != 0)
        rt = new DataValueLong(left->GetLong() / r);
    } else if (left->IsDigital() && right->IsDigital()) {
      double r = right->GetDouble();
      if (r != 0)
        rt = new DataValueDouble(left->GetDouble() / r);
    }
    if (rt == nullptr) {
      rt = left->Clone();
    }

    if (!left->IsReuse())
      delete left;
    if (!right->IsReuse())
      delete right;
    return rt;
  }

public:
  ExprData *_exprLeft;
  ExprData *_exprRight;
};
} // namespace storage
