#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
#include <string>

class Database {
    sqlite3* db;

    void create_tables();

public:
    Database(std::string filename);
    Database(const Database&) = delete; // No copy allowed
    Database(Database&&) = delete; // No move allowed

    ~Database();
};

#endif