from enum import Enum

class VariableType(Enum):
    INT8 = ("int8_t", "sqlite3_bind_int", "INTEGER", 1)
    INT16 = ("int16_t", "sqlite3_bind_int", "INTEGER", 2)
    INT32 = ("int32_t", "sqlite3_bind_int", "INTEGER", 4)
    INT64 = ("int64_t", "sqlite3_bind_int64", "INTEGER", 8)

    UINT8 = ("uint8_t", "sqlite3_bind_int", "INTEGER", 1)
    UINT16 = ("uint16_t", "sqlite3_bind_int", "INTEGER", 2)
    UINT32 = ("uint32_t", "sqlite3_bind_int", "INTEGER", 4)
    UINT64 = ("uint64_t", "sqlite3_bind_int64", "INTEGER", 8)

    FLOAT = ("float", "sqlite3_bind_double", "REAL", 4)
    DOUBLE = ("float", "sqlite3_bind_double", "REAL", 8)

    def __init__(self, typename: str, sqlite_bind: str, sql_type: str, size: int, default_value=0):
        self.typename = typename
        self.sqlite_bind = sqlite_bind
        self.sql_type = sql_type
        self.size = size
        self.default_value = 0