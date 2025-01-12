#include "DatabaseManager.h"

namespace storage {
const uint32_t DatabaseManager::FAST_SIZE = 127;
static inline Database *GenNullPointer() {
  Database *db =
      (Database *)new char[sizeof(Database *) * DatabaseManager::FAST_SIZE];
  memset(db, 0, sizeof(Database *) * DatabaseManager::FAST_SIZE);
  return db;
}

MTreeMap<MString, Database *> DatabaseManager::_mapDb;
SpinMutex DatabaseManager::_spinMutex;
Database *DatabaseManager::_fastDbCache = GenNullPointer();

bool DatabaseManager::LoadDb() { return true; }

bool DatabaseManager::AddDb() { return true; }

bool DatabaseManager::DelDb() { return true; }

bool DatabaseManager::ListDb(MVector<MString> &vctDb) { return true; }

Database *DatabaseManager::FindDb(MString db) { return nullptr; }

} // namespace storage