from dataclasses import dataclass

@dataclass
class DataType:
    c_type: str
    """primitive type in C that represents this object"""

    num_bytes: int 
    """the number of bytes this datatype takes up when encoded"""

    encoding_method: str
    """
    A format string that represents a c method to be called to encode the c_type into the packet
    This format string will always have exactly two parameters (in this order):
     1. Variable name of the pointer to a char* buffer where the encoded item should be stored. 
     2. The variable name of the native c type to encode
    """

    decoding_method: str
    """
    A format string that represents a c method to be called to decode the c_type from the packet
    This format string will always have exactly two parameters (in this order): 
     1. The variable name of the native c type where the decoded output should be stored
     2. Variable name of the pointer to a char* buffer where the encoded item should be read. 
    """

def parse_type(type_data: str | dict) -> DataType:
    if isinstance(type_data, dict) and 'name' not in type_data:
        raise ValueError(f'Provided type specification does not provide a name: {type_data}')
    if isinstance(type_data, str):
        type_data = {'name': type_data}
    
    if (res := try_parse_int(type_data)) is not None: return res
    if (res := try_parse_float(type_data)) is not None: return res

    raise ValueError(f'Unknown type specification provided: {type_data}')

def try_parse_float(type_data: dict) -> DataType | None:
    dname = type_data['name'].lower()

    if len(type_data) == 1: # Simple floating point types
        if dname in ['float32', 'float32_t', 'float_32', 'f32', 'float']:
            return DataType(
                c_type='float', 
                num_bytes=4, 
                encoding_method='memcpy(%s, &(%s), 4)',
                decoding_method='memcpy(&(%s), %s, 4)'
            )
        if dname in ['float64', 'float64_t', 'float_64', 'f64', 'double']:
            return DataType(
                c_type='double',
                num_bytes=8,
                encoding_method='memcpy(%s, &(%s), 8)',
                decoding_method='memcpy(&(%s), %s, 8)'
            )

    for bits in [8, 16, 32, 64]:
        if dname in [f'qfloat{bits}', f'qfloat{bits}_t', f'qfloat{bits}', f'q{bits}']:
            if len(type_data) == 3:
                mn, mx = type_data.get('min', None), type_data.get('max', None)
            elif len(type_data) == 2:
                mx = type_data.get('max', None)
                mn = -mx if mx is not None else None
            else: continue
            if mx is None or mn is None: continue

            return DataType(
                c_type='double' if bits >= 32 else 'float',
                num_bytes=bits // 8,
                encoding_method=f'quantize_float_{bits}(%s, %s, {mn}, {mx})',
                decoding_method=f'unquantize_float_{bits}(&(%s), %s, {mn}, {mx})'
            )

    return None

def try_parse_int(type_data: dict) -> DataType | None:
    if len(type_data) > 1:
        return None # Integer types only have one specification field (name)
    dname = type_data['name'].lower()

    for bits in [8, 16, 32, 64]:
        if dname in [f'int{bits}', f'int{bits}_t', f'int_{bits}', f'i{bits}']:
            return DataType(
                c_type=f'int{bits}_t', 
                num_bytes=(bits // 8), 
                encoding_method=f'memcpy(%s, &(%s), {bits // 8})',
                decoding_method=f'memcpy(&(%s), %s, {bits // 8})'
            )
        elif dname in [f'uint{bits}', f'uint{bits}_t', f'uint_{bits}', f'u{bits}']:
            return DataType(
                c_type=f'uint{bits}_t', 
                num_bytes=(bits // 8), 
                encoding_method=f'memcpy(%s, &(%s), {bits // 8})',
                decoding_method=f'memcpy(&(%s), %s, {bits // 8})',
            )

    return None