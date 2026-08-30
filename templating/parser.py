import jinja2
from pyjson5 import decode
from dataclasses import dataclass, asdict

from datatypes import DataType, parse_type
from name import Name

@dataclass
class DataPacketField:
    name: Name
    datatype: DataType

@dataclass 
class DataPacket:
    name: Name
    fields: list[DataPacketField]
    encoded_length: int
    """Length of the encoded data packet payload in bytes"""

    @staticmethod
    def from_dict(spec: dict):
        assert 'name'  in spec, f'Packet specification did not contain name: {spec}'
        assert 'attributes' in spec, f'Packet specification did not contain any attributes'
        assert isinstance(spec['attributes'], dict) and len(spec['attributes']) > 0, \
            f'Packet specification contained empty attribute list'
        fields = sorted([
            DataPacketField(name=Name.from_str(name), datatype=parse_type(data_type)) 
            for name, data_type in spec['attributes'].items()
        ], key=lambda x: (x.datatype.num_bytes, x.name.english_name))
        return DataPacket(
            name = Name.from_str(spec['name']), 
            fields = fields, 
            encoded_length=sum(i.datatype.num_bytes for i in fields)
        )
        

# Generate specification
with open('config.jsonc', 'r') as f:
    raw_json = decode(f.read())

data_packets = [DataPacket.from_dict(i) for i in raw_json['data_packets']]

env = jinja2.Environment(trim_blocks=True, lstrip_blocks=True)
for file in ['gs_core.c', 'gs_core.h']:
    with open(f"templating/templates/{file}.jinja") as f:
        template_str = f.read()
    template = env.from_string(template_str)

    # Pass data and render the result
    output = template.render(data_packets=[asdict(i) for i in data_packets])

    with open(f'core/{file}', 'w+') as f:
        f.write(output)