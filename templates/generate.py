from jinja2 import Environment, FileSystemLoader
from variables import VariableType
from data_packets import DataPacket
import os

TARGETS = [
    "include/database/sql_database.h",
    "source/database/sql_database.cpp",
    "include/globals/datatypes.h",
    "source/globals/datatypes.cpp",
    "source/web_server/json_parser.cpp",
    "include/web_server/json_parser.h"
]

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

    environment = Environment(loader=FileSystemLoader("./"), trim_blocks=True, lstrip_blocks=True)
    environment.filters['strformat'] = lambda v, fmt: fmt.format(v) # Allows for a format-string filter

    for target in TARGETS:
        _, template_name = os.path.split(target)
        template = environment.get_template(os.path.join("templates/templates", template_name + ".jinja"))

        content = template.render(data)

        with open(target, "w+") as f:
            f.write(content)