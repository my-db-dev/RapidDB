﻿#pragma once
#include "../cache/StrBuff.h"
#include "../config/ErrorID.h"
#include "../dataType/DataValueDigit.h"
#include "../dataType/DataValueFixChar.h"
#include "../dataType/DataValueVarChar.h"
#include "../dataType/IDataValue.h"
#include "../table/Table.h"
#include "../utils/ErrorMsg.h"
#include "BaseExpr.h"
#include "ExprAggr.h"
#include "ExprData.h"
#include "ExprLogic.h"
#include <unordered_set>

using namespace std;
namespace storage {
enum class TableType {
  PersistTable, // From physical table
  CacheTable,   // The result saved in temp block table
  HashTable     // The result saved in hash table
};

enum class JoinType { INNER_JOIN, LEFT_JOIN, RIGHT_JOIN, OUTTER_JOIN };

// The column order by
struct OrderCol {
  int colPos; // The position of column for order
  bool bAsc;  // True: asc; False desc
};

// To query physical table, point out which index will be used. Only one index
// can be selected.
struct SelIndex {
  // If the primary or secondary index can be used to query, copy the query
  // conditions to here. Only one index can be used. Only valid for physical
  // table select
  ExprLogic *_indexExpr;
  // which index used, The position of index that start from 0(primary key)
  int _indexPos;
};

struct PhyTableInfo {
  PhyTableInfo(uint32_t tid, string name, DT_MilliSec lastUpdate)
      : _tid(tid), _tName(name), _dtLastUpdate(lastUpdate) {}
  // Table id, from PhysTable::_tid
  uint32_t _tid;
  // Table name
  string _tName;
  // The last date time to update this table
  DT_MilliSec _dtLastUpdate;
};

// Base class for all statement
class ExprStatement : public BaseExpr {
}
// Base class for all select
class ExprSelect : public ExprStatement {
public:
  ExprSelect(ExprTable *destTable, ExprLogic *where,
             MVector<OrderCol>::Type *vctOrder, bool bDistinct, int offset,
             int rowCount, bool bCacheResult, VectorDataValue *paraTmpl,
             ExprStatement *parent)
      : _destTable(destTable), _where(where), _vctOrder(vctOrder),
        _offset(offset), _rowCount(rowCount), _bDistinct(bDistinct),
        _bCacheResult(bCacheResult), _paraTmpl(paraTmpl), _parent(parent) {}
  ~ExprSelect() {
    delete _destTable;
    delete _where;
    delete _vctOrder;
    delete _paraTmpl;
  }

  ExprTable *GetDestTable() { return _destTable; }
  ExprLogic *GetWhere() { return _where; }
  MVector<OrderCol>::Type *GetVctOrder() { return _vctOrder; }
  int GetOffset() { return _offset; }
  int GetRowCount() { return _rowCount; }
  bool IsDistinct() { return _bDistinct; }
  bool IsCacheResult() { return _bCacheResult; }
  const VectorDataValue *GetParaTemplate() { return _paraTmpl; }
  ExprStatement *GetParent() { return _parent; }

protected:
  // Parent statement for this select statement
  ExprStatement *_parent;
  ExprTable *_destTable; // The columns of selected result
  ExprLogic *_where;     // The where conditions
  // The columns idx and order type for order by, NO order with nullptr
  MVector<OrderCol>::Type *_vctOrder;
  // offset for return rows, default 0. Only valid for top select result.
  int _offset;
  // The max rows to return, -1 means return all. Only valid for top select
  // result.
  int _rowCount;
  bool _bDistinct; // If remove repeated rows
  // If cache result for future query, only valid for top select result.
  bool _bCacheResult;
  // The template for paramters, only need by top statement
  VectorDataValue *_paraTmpl;
};

class ExprTableSelect : public ExprSelect {
public:
  ExprTableSelect(PhysTable *physTable, ExprTable *destTable, ExprLogic *where,
                  SelIndex *selIndex, MVector<OrderCol>::Type *vctOrder,
                  bool bDistinct, int offset, int rowCount, bool bCacheResult,
                  VectorDataValue *paraTmpl, ExprStatement *parent)
      : ExprSelect(destTable, where, vctOrder, bDistinct, offset, rowCount,
                   bCacheResult, paraTmpl, parent),
        _physTableInfo(physTable->TableID(), physTable->GetTableName(),
                       physTable->GetLastUpdateTime()),
        _selIndex(selIndex) {}
  ~ExprTableSelect() { delete _selIndex; }

  ExprType GetType() { return ExprType::EXPR_TABLE_SELECT; }
  PhyTableInfo &GetSourTableInfo() const { return _physTableInfo; }
  const SelIndex *GetSelIndex() const { return _selIndex; }

protected:
  // The source table information.
  PhyTableInfo _physTableInfo;
  // Which index used to search. Null means traverse all table.
  SelIndex *_selIndex;
};

class ExprInsert : public ExprStatement {
public:
  ExprInsert(PhysTable *phyTable, ExprTable *exprTable,
             VectorDataValue *paraTmpl, ExprSelect *exprSelect = nullptr,
             bool bUpsert = false)
      : _physTableInfo(physTable->TableID(), physTable->GetTableName(),
                       physTable->GetLastUpdateTime()),
        _exprTable(exprTable), _exprSelect(exprSelect), _bUpsert(bUpsert),
        _paraTmpl(paraTmpl) {}

  ~ExprInsert() {
    delete _exprTable;
    delete _exprSelect;
    delete _paraTmpl;
  }

  ExprType GetType() { return ExprType::EXPR_INSERT; }
  PhyTableInfo &GetSourTableInfo() const { return _physTableInfo; }
  const ExprTable *GetExprTable() { return _exprTable; }
  const ExprSelect *GetExprSelect() { return _exprSelect; }
  bool IsUpsert() { return _bUpsert; }
  const VectorDataValue *GetParaTemplate() { return _paraTmpl; }

protected:
  // The destion physical table information
  PhyTableInfo _physTableInfo;
  // The expression that how to calc values from input or select
  ExprTable *_exprTable;
  // Used for insert into TABLE A select from TABLE B
  ExprSelect *_exprSelect;
  // True, update if the key has exist
  bool _bUpsert;
  // The template for paramters, only need by top statement
  VectorDataValue *_paraTmpl;
};

class ExprUpdate : public ExprStatement {
public:
  ExprUpdate(PhysTable *phyTable, ExprTable *exprTable, ExprLogic *where,
             SelIndex *selIndex, VectorDataValue *paraTmpl)
      : _physTableInfo(physTable->TableID(), physTable->GetTableName(),
                       physTable->GetLastUpdateTime()),
        _exprTable(exprTable), _where(where), _paraTmpl(paraTmpl),
        _selIndex(selIndex) {}

  ~ExprUpdate() {
    delete _exprTable;
    delete _where;
    delete _paraTmpl;
  }

  ExprType GetType() { return ExprType::EXPR_UPDATE; }
  PhyTableInfo &GetSourTableInfo() const { return _physTableInfo; }
  const ExprTable *GetExprTable() { return _exprTable; }
  const ExprLogic *GetWhere() { return _where; }
  const VectorDataValue *GetParaTemplate() { return _paraTmpl; }
  const SelIndex *GetSelIndex() const { return _selIndex; }

protected:
  // The destion physical table information
  PhyTableInfo _physTableInfo;
  // The expression that how to calc values
  ExprTable *_exprTable;
  // Where condition
  ExprLogic *_where;
  // The template for paramters, only need by top statement
  VectorDataValue *_paraTmpl;
  // Which index used to search. Null means traverse all table.
  SelIndex *_selIndex;
};

class ExprDelete : public ExprStatement {
public:
  ExprDelete(PhysTable *phyTable, ExprLogic *where, SelIndex *selIndex,
             VectorDataValue *paraTmpl)
      : _physTableInfo(physTable->TableID(), physTable->GetTableName(),
                       physTable->GetLastUpdateTime()),
        _where(where), _paraTmpl(paraTmpl), _selIndex(selIndex) {}
  ~ExprDelete() {
    delete _where;
    delete _paraTmpl;
  }

  ExprType GetType() { return ExprType::EXPR_DELETE; }
  PhyTableInfo &GetSourTableInfo() const { return _physTableInfo; }
  const ExprLogic *GetWhere() { return _where; }
  const VectorDataValue *GetParameters() { return _paraTmpl; }
  const SelIndex *GetSelIndex() const { return _selIndex; }

protected:
  // The destion persistent table information
  PhyTableInfo _physTableInfo;
  // Where condition
  ExprLogic *_where;
  // The template for paramters, only need by top statement
  VectorDataValue *_paraTmpl;
  // Which index used to search. Null means traverse all table.
  SelIndex *_selIndex;
};

// class ExprJoin : public ExprSelect {
// public:
//   ExprJoin(BaseTable *sourTable, BaseTable *destTable,
//            ExprValueArrayOut *exprVAout, ExprCondition *where,
//            MVector<OrderCol>::Type &vctOrder, bool bDistinct, bool
//            bCacheResult, BaseTable *rightTable, ExprOn *exprOn, JoinType
//            jType, VectorDataValue &vctPara)
//       : ExprSelect(sourTable, destTable, exprVAout, where, vctOrder,
//       bDistinct,
//                    bCacheResult, vctPara),
//         _rightTable(rightTable), _exprOn(exprOn), _joinType(jType) {}
//   ~ExprJoin() {
//     delete _rightTable;
//     delete _exprOn;
//   }

//   ExprType GetType() { return ExprType::EXPR_JOIN; }
//   const BaseTable *GetRightTable() const { return _rightTable; }
//   const ExprOn *GetExprOn() const { return _exprOn; }
//   JoinType GetJoinType() { return _joinType; }

// protected:
//   // The right source table information for join
//   BaseTable *_rightTable;
//   // On condition for join
//   ExprOn *_exprOn;
//   // Join type
//   JoinType _joinType;
// };

// class ExprGroupBy : public ExprSelect {
// public:
//   ExprGroupBy(BaseTable *sourTable, BaseTable *destTable,
//               MVector<ExprAggr *>::Type &vctAggr, ExprCondition *where,
//               MVector<OrderCol>::Type &vctOrder, bool bDistinct,
//               bool bCacheResult, ExprCondition *having,
//               VectorDataValue &vctPara)
//       : ExprSelect(sourTable, destTable, nullptr, where, vctOrder, bDistinct,
//                    bCacheResult, vctPara),
//         _having(having) {
//     _vctChild.swap(vctAggr);
//   }
//   ~ExprGroupBy() {
//     for (auto child : _vctChild)
//       delete child;

//     _vctChild.clear();
//     delete _having;
//   }

//   ExprType GetType() { return ExprType::EXPR_GROUP_BY; }
//   MVector<ExprAggr *>::Type GetVctChild() { return _vctChild; }
//   ExprCondition *GetHaving() { return _having; }

// protected:
//   // This variable will replace _exprVAout to calc the values.
//   MVector<ExprAggr *>::Type _vctChild;
//   ExprCondition *_having;
// };
} // namespace storage
