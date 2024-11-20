import re
from utils.exceptions import VariableNameInvalidException
from typing import NoReturn

VARIABLE_NAME_REGEX_PATTERN = '^[a-zA-Z_][a-zA-Z_0-9 ]*$'

def generate_valid_snake_variable_name(name: str) -> str | NoReturn:
    if re.match(VARIABLE_NAME_REGEX_PATTERN, name) is None:
        raise VariableNameInvalidException(
            name, 
            "Variable name must start with a letter or underscore, and may only contain letters, numbers, or underscores"
        )
    
    return name.lower().replace(' ', '_')

def generate_valid_camel_variable_name(name: str) -> str | NoReturn:
    if re.match(VARIABLE_NAME_REGEX_PATTERN, name) is None:
        raise VariableNameInvalidException(
            name, 
            "Variable name must start with a letter or underscore, and may only contain letters, numbers, or underscores"
        )
    
    return name.title().replace(' ', '')