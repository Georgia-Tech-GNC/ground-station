import re
from utils.exceptions import VariableNameInvalidException

VARIABLE_NAME_REGEX_PATTERN = '^[a-zA-Z][a-zA-Z_0-9 ]*$'

def generate_snake_camel_name_pair(name: str) -> tuple[str, str]:
    """
    Given a string input name, produces two strings:
    - The first is a valid variable name in the form word_word_word
    - The second is a valid struct name in the form WordWordWord

    If the name is invalid, it will raise a VariableNameInvalidException.
    A valid name starts with a letter, and contains only letters, numbers, and underscores
    It is assumed that words are separated by spaces, not underscores
    """
    if re.match(VARIABLE_NAME_REGEX_PATTERN, name) is None:
        raise VariableNameInvalidException(
            name,
            "Variable name must start with a letter, and may only contain letters, numbers, or underscores"
        )

    return name.lower().replace(' ', '_'), name.title().replace(' ', '')