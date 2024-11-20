import json
from parse_types.command import Command

with open('sample_data.json', 'r') as file:
    data = dict(json.load(file))

    print(data)

    for i, (item, value) in enumerate(data['Commands'].items()):
        flags = [] if value is None else value
        print(repr(Command(i+1, item, *flags)))