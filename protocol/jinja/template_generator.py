from jinja2 import Environment, FileSystemLoader
import json

FLOAT_TYPES = ['float']
INT_TYPES = [f'{h}int{x}_t' for h in ('u', '') for x in (8, 16, 32)]

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

environment = Environment(loader=FileSystemLoader("./templates/"), trim_blocks=True)
environment.globals['duplicate'] = duplicate
environment.globals['printf_sql_type'] = PRINTF_SQL_TYPE
environment.globals['upper_plus_suffix'] = upper_plus_suffix
environment.globals['get_sql_type'] = get_sql_type

htemplate = environment.get_template("protocol_header_template.j2")
ctemplate = environment.get_template("protocol_c_template.j2")

H_FILE_LOCATIONS = ["../includes/protocol.h", "../../ground-station-core/lib/protocol/protocol.h"]
C_FILE_LOCATIONS = ["../src/protocol.c", "../../ground-station-core/lib/protocol/protocol.c"]

DATA = {}
with open("data.json") as f:
    DATA = json.load(f)

def cvt_type_to_max_len_str(i):
    if i in FLOAT_TYPES: return "1.1234567890E-38"
    elif i in INT_TYPES: return "-2147483647"
    else: raise Exception(f"Invalid type {i}")

for filepath in H_FILE_LOCATIONS:
    with open(filepath, "w") as f:
        # Calculate max sql write size (THIS IS A STRING CHARACTER SIZE! not bits!)
        max_sql_write_size = 5
        for name, message in DATA["DATA_MESSAGES"].items():
            size = length = len(f"\"INSERT INTO {name} VALUES ({", ".join(["'" + cvt_type_to_max_len_str(i) + "'" for i in message.values()])}, 'YYYY-MM-DD HH-MI-SS');\"")
            max_sql_write_size = max(max_sql_write_size, size)

        DATA['MAX_SQL_WRITE_SIZE'] = max_sql_write_size + 10 # Extra 10 just in case

        f.write(htemplate.render(DATA))

for filepath in C_FILE_LOCATIONS:
    with open(filepath, "w") as f:
        f.write(ctemplate.render(DATA))


