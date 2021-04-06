﻿#pragma once
namespace storage {
enum ErrorID {
  TB_INVALID_FILE_VERSION = 1001,
  TB_INVALID_TABLE_NAME = 1002,
  TB_REPEATED_COLUMN_NAME = 1003,
  TB_Array_MAX_LEN = 1004,
  TB_COLUMN_OVER_LENGTH = 1005,
  TB_REPEATED_INDEX = 1006,
  TB_UNEXIST_COLUMN = 1007,
  TB_INDEX_UNSUPPORT_DATA_TYPE = 1008,
  TB_INDEX_EMPTY_COLUMN = 1009,
  TB_ERROR_INDEX_VERSION = 1010,
  TB_ERROR_RECORD_VERSION_STAMP = 1011,
  TB_COLUMN_INDEX_OUT_RANGE = 1012,
  TB_INVALID_COLUMN_NAME = 1013,
  TB_INVALID_RESULT_SET = 1014,

  DT_UNSUPPORT_CONVERT = 2001,
  DT_INPUT_OVER_LENGTH = 2002,
  DT_UNKNOWN_TYPE = 2003,

  CM_EXCEED_LIMIT = 3001,
  CM_FAILED_FIND_SIZE = 3002,

  FILE_OPEN_FAILED = 4001,

  CORE_EXCEED_KEY_LENGTH = 5001,
  CORE_REPEATED_RECORD = 5002
};
}
