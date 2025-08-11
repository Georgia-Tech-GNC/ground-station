from jinja2 import Environment, FileSystemLoader
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

    def __init__(self, typename: str, sqlite_bind: str, sql_type: str, size: int):
        self.typename = typename
        self.sqlite_bind = sqlite_bind
        self.sql_type = sql_type
        self.size = size

class DataPacket:

    def __init__(self, name: str, variables: dict[str, VariableType]):
        self.title_name = name.title().replace(' ', '')
        self.variables = [
            {'name': var_name, 'sqlite_binding': var_type.sqlite_bind, 'length': var_type.size} 
            for var_name, var_type in variables.items()
        ]
        self.byteform = {'length': sum([var_type.size for var_type in variables.values()])}
        self.variables_by_type = {}
        for var_name, var_type in variables.items():
            if var_type.typename not in self.variables_by_type: 
                self.variables_by_type[var_type.typename] = []
            self.variables_by_type[var_type.typename].append(var_name)
        self.sqlite_prepared_statement = f"INSERT INTO {self.title_name} VALUES ({", ".join(["?"] * len(variables))})"
        self.sql_table_gen_command = f"CREATE TABLE IF NOT EXISTS {self.title_name} (timestamp INTEGER PRIMARY KEY, { \
            ', '.join([f'{var_name} {var_type.sql_type}' for var_name, var_type in variables.items()]) \
        })"

if __name__ == "__main__":
    data = {
        "all_data_packets": [
            DataPacket("rocket kinematics data", {
                "x": VariableType.FLOAT,
                "y": VariableType.FLOAT,
                "z": VariableType.FLOAT,
                "roll": VariableType.UINT16,
                "pitch": VariableType.UINT16
            }).__dict__,   

            DataPacket("rocket sensor data", {
                "gps_x": VariableType.INT64,
                "gps_y": VariableType.INT32,
                "gps_z": VariableType.INT32,
            }).__dict__,
        ]
    }

    FILES = ["datatypes.h", "datatypes.cpp"]
    environment = Environment(loader=FileSystemLoader("./"), trim_blocks=True, lstrip_blocks=True)

    for file_name in FILES:
        template = environment.get_template(file_name + ".jinja")

        content = template.render(data)

        with open(file_name, "w+") as f:
            f.write(content)