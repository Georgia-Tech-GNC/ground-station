#include <exception>

#include "database.h"
#include "datatypes.h"

Database::Database(std::string filename) {
    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Generating database file failed. "
            "This is most likely an internal error with Sqlite3, although it may occur due to a malformed database filename."
        );
    };
    if (!build_all_tables(db)) {
        throw std::runtime_error("Building database tables failed. "
            "Please inspect table generation code in \"datatypes.h\" or the available heap allocation size."
        );
    }
}

Database::~Database() {
    sqlite3_close(db);
}