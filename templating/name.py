from dataclasses import dataclass
import re

VALID_C_NAME_REGEX = '^[a-zA-Z_][a-zA-Z0-9_]*$'
C_KEYWORDS = {
    'alignas', 'alignof' 'auto', 'bool', 'break', 'case', 'char', 'const', 'constexpr', 
    'continue', 'default', 'do', 'double', 'else', 'enum', 'extern', 'false', 'float', 
    'for', 'goto', 'if', 'inline', 'int', 'long', 'nullptr', 'register', 'restrict', 
    'return', 'short', 'signed', 'sizeof', 'static', 'static_assert', 'struct', 'switch', 
    'thread_local', 'true', 'typedef', 'typeof', 'typeof_unqual', 'union', 'unsigned', 
    'void', 'volatile', 'while', '_Atomic', '_BitInt', '_Complex', '_Decimal128', 
    '_Decimal32', '_Decimal64', '_Generic', '_Imaginary', 
}

@dataclass
class Name:
    english_name: str
    c_name: str

    @staticmethod
    def from_str(name: str):
        c_name = '_'.join(name.lower().strip().split())
        assert c_name not in C_KEYWORDS and re.match(VALID_C_NAME_REGEX, c_name) is not None, \
            f'Generated c_name ({c_name}) was not valid'
        return Name(english_name=name, c_name=c_name)
