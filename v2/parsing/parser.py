import json
import re
from enum import Enum
from dataclasses import dataclass
from jinja2 import Environment, FileSystemLoader

class Name:
    VALID_NAME_REGEX_PATTERN = '^[a-zA-Z][a-zA-Z0-9 ]*$'
    def __init__(self, raw_name: str, all_names_set: set):
        if not isinstance(raw_name, str):
            raise TypeError(f"Passed in raw_name of type {type(raw_name)}, expected string")
        elif re.match(Name.VALID_NAME_REGEX_PATTERN, raw_name) is None:
            raise ValueError(f"Passed name {raw_name} is invalid: names must start with a letter and contain only letters, numbers, and spaces")
        elif raw_name.lower() in all_names_set:
            raise ValueError(f"Duplicated name {raw_name}! Note that duplicate names are case insensitive")
        
        all_names_set.add(raw_name.lower())

        self.__snake_case = raw_name.lower().replace(' ', '_')
        self.__title_case = raw_name.title().replace(' ', '')
        self.__raw_name = raw_name

    @property
    def lower_snake_case(self) -> str:
        return self.__snake_case

    @property
    def upper_snake_case(self) -> str:
        return self.__snake_case.upper()
    
    @property
    def title_case(self) -> str:
        return self.__title_case
    
    @property
    def raw_name(self) -> str:
        return self.__raw_name
    
    def __str__(self) -> str:
        return self.raw_name
    
    def __repr__(self) -> str:
        return self.raw_name
    

@dataclass
class LanguageDataNames:
    python: str
    c: str
    sqlite: str
    sqlite_bind_method_name: str

class DataType(Enum):
    UINT_8T  = LanguageDataNames('int', 'uint8_t', 'INTEGER', 'sqlite3_bind_int')
    UINT_16T = LanguageDataNames('int', 'uint16_t', 'INTEGER', 'sqlite3_bind_int')
    UINT_32T = LanguageDataNames('int', 'uint32_t', 'INTEGER', 'sqlite3_bind_int64') 
    INT_8T   = LanguageDataNames('int', 'int8_t', 'INTEGER', 'sqlite3_bind_int')
    INT_16T  = LanguageDataNames('int', 'int16_t', 'INTEGER', 'sqlite3_bind_int')
    INT_32T  = LanguageDataNames('int', 'int32_t', 'INTEGER', 'sqlite3_bind_int')
    FLOAT_32T = LanguageDataNames('float', 'float', 'FLOAT', 'sqlite3_bind_double')
    FLOAT_16T = LanguageDataNames('float', 'float', 'FLOAT', 'sqlite3_bind_double')


def get_data_type_from_str(string: str) -> LanguageDataNames:
    if not isinstance(string, str):
        raise TypeError(f"Recieved type {type(string)} for parameter string, expected str")
    
    match string.lower().replace(' ', '').replace('_', ''):
        case "uint8" | "uint8t":
            return DataType.UINT_8T.value
        case "uint16" | "uint16t":
            return DataType.UINT_16T.value
        case "uint32" | "uint32t":
            return DataType.UINT_32T.value
        case "int8" | "int8t":
            return DataType.INT_8T.value
        case "int16" | "int16t":
            return DataType.INT_16T.value
        case "int32" | "int32t":
            return DataType.INT_32T.value
        case "float16" | "float16t":
            return DataType.FLOAT_16T.value
        case "float32" | "float32t":
            return DataType.FLOAT_32T.value
        case _:
            raise ValueError(f"Type {string.lower()} is not a valid supported type")
        

with open('sample.json', 'r') as file:
    data = dict(json.load(file))

    TOP_LEVEL_NAMES = set()

    packets = {}
    
    for item, value in data['Commands'].items():
        FLAG_NAMES = set() 
        flags = [] if value is None else [Name(x, FLAG_NAMES) for x in value]
        print(Name(item, TOP_LEVEL_NAMES), flags)

    for data_packet_name, contents in data['Data Packets'].items():
        name = Name(data_packet_name, TOP_LEVEL_NAMES)
        PARAM_NAMES = set()
        params = {}
        for param_name, param_type in contents.items():
            params[Name(param_name, PARAM_NAMES)] = get_data_type_from_str(param_type)

        packets[name] = params

    print(packets)

    sql_generate_table_commands = [f"CREATE TABLE {name.title_case} ({", ".join([
        f"{param_name.title_case} {param_type.sqlite}" for param_name, param_type in content.items()
    ])});" for name, content in packets.items()]

    sql_generate_prepared_statement_commands = [f"INSERT INTO {name.title_case} VALUES ({", ".join([
        "?" for _ in content
    ])});" for name, content in packets.items()]
    sql_generate_prepared_statement_commands = {x: len(x) + 1 for x in sql_generate_prepared_statement_commands}

    print(sql_generate_table_commands)
    print(sql_generate_prepared_statement_commands)

environment = Environment(loader=FileSystemLoader("../templates/"))
template = environment.get_template("sql_executer_c.jinja")
template2 = environment.get_template("sql_executer_h.jinja")

with open('../templates/output.c', 'w+') as f:
    f.write(template.render({'NUM_DATA_PACKET_TYPES': len(sql_generate_table_commands), 'sql_prepared_statement_commands': sql_generate_prepared_statement_commands}))

with open('../templates/output1.c', 'w+') as f:
    f.write(template2.render({'DATA_PACKETS': packets, 'NUM_DATA_PACKET_TYPES': len(sql_generate_table_commands), 'sql_prepared_statement_commands': sql_generate_prepared_statement_commands}))