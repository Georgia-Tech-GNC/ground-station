from jinja2 import Environment, FileSystemLoader
import json

FLOAT_TYPES = ['float']
INT_TYPES = [f'{h}int{x}_t' for h in ('u', '') for x in (8, 16, 32)]
SIZES = {
    'float': 32/8,
    'uint8_t': 1,
    'uint16_t': 2,
    'uint32_t': 4,
    'int8_t': 1,
    'int16_t': 2,
    'int32_t': 4,
    'int64_t': 8
}

""" Extra stuff
                "accel_bias_x": "float",
                "accel_bias_y": "float",
                "accel_bias_z": "float",
                "gyro_bias_x": "float",
                "gyro_bias_y": "float",
                "gyro_bias_z": "float",
                "gps_offset_x": "float",
                "gps_offset_y": "float",
                "gps_offset_z": "float",
                "barometer_offset": "float"
"""

def PRINTF_SQL_TYPE(l):
    r = []
    for x in l:
        if x in FLOAT_TYPES: r.append("'%.10E'")
        elif x in INT_TYPES: r.append("'%d'")
        else: raise Exception(f"Invalid type {x}")

    return r

def get_sql_type(x):
    if x in FLOAT_TYPES: return 'real'
    elif x in INT_TYPES: return 'int'
    else: raise Exception(f"Invalid type {x}")

def upper_plus_suffix(l, s):
    return [i.upper() + s for i in l]

def duplicate(s, i):
    return [s for _ in range(len(i))]

def size_of_datatype(datatype: str) -> int:
    return SIZES[datatype]

environment = Environment(loader=FileSystemLoader("./templates/"), trim_blocks=True)
environment.globals['duplicate'] = duplicate
environment.globals['printf_sql_type'] = PRINTF_SQL_TYPE
environment.globals['upper_plus_suffix'] = upper_plus_suffix
environment.globals['get_sql_type'] = get_sql_type

templates = {
    environment.get_template("protocol_header_template.j2"): ["../includes/protocol.h", "../../ground-station-core/lib/protocol/protocol.h"],
    environment.get_template("protocol_c_template.j2"): ["../src/protocol.c", "../../ground-station-core/lib/protocol/protocol.c"]
}

DATA = {}
with open("data.json") as f:
    DATA = json.load(f)

def cvt_type_to_max_len_str(i):
    if i in FLOAT_TYPES: return "1.1234567890E-38"
    elif i in INT_TYPES: return "-2147483647"
    else: raise Exception(f"Invalid type {i}")

for template, paths in templates.items():
    data_message_sizes = {}
    max_sql_write_size = 5
    for name, message in DATA["DATA_MESSAGES"].items():
        values = ", ".join(["'" + cvt_type_to_max_len_str(i) + "'" for i in message.values()])
        size = length = len(f"\"INSERT INTO {name} VALUES ({values}, 'YYYY-MM-DD HH-MI-SS');\"")
        max_sql_write_size = max(max_sql_write_size, size)
        data_message_sizes[name] = sum(map(size_of_datatype, message.values())) + 8

    DATA['MAX_SQL_WRITE_SIZE'] = max_sql_write_size + 10 # Extra 10 just in case
    DATA['DATA_MESSAGE_SIZES'] = data_message_sizes
    DATA['SIZES'] = SIZES
    text = template.render(DATA)
    for path in paths:
        with open(path, "w") as f:
            print(f'Writing to {path}')
            f.write(text)
