from enum import Enum
from collections import namedtuple

class VariableTypeMixin(
    namedtuple('VariableTypeMixin', [
        'typename', 'sqlite_bind', 'sql_type', 'size', 'printf_flag', 'max_str_len', 'default_value',
    ])
):
    def __new__(
        cls,
        typename: str, sqlite_bind: str, sql_type: str, size: int, 
        printf_flag: str, max_str_len: int,
        default_value=0
    ):
        return super().__new__(cls, typename, sqlite_bind, sql_type, size, printf_flag, max_str_len, default_value)

class VariableType(VariableTypeMixin, Enum):
    INT8 = ("int8_t", "sqlite3_bind_int", "INTEGER", 1, "%hhd", len("-128"))
    INT16 = ("int16_t", "sqlite3_bind_int", "INTEGER", 2, "%hd", len("-32768"))
    INT32 = ("int32_t", "sqlite3_bind_int", "INTEGER", 4, "%d", len("-2147483648"))
    INT64 = ("int64_t", "sqlite3_bind_int64", "INTEGER", 8, "%lld", len("-9223372036854775808"))

    UINT8 = ("uint8_t", "sqlite3_bind_int", "INTEGER", 1, "%hhu", len("256"))
    UINT16 = ("uint16_t", "sqlite3_bind_int", "INTEGER", 2, "%hu", len("65536"))
    UINT32 = ("uint32_t", "sqlite3_bind_int", "INTEGER", 4, "%u", len("4294967296"))
    UINT64 = ("uint64_t", "sqlite3_bind_int64", "INTEGER", 8, "%llu", len("18446744073709551616"))

    # Maximum string sizes for decimals are capped at 40 characters by mongoose.
    FLOAT = ("float", "sqlite3_bind_double", "REAL", 4, "%g", 40)
    DOUBLE = ("float", "sqlite3_bind_double", "REAL", 8, "%g", 40)