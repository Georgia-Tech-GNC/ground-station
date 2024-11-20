

class VariableNameInvalidException(Exception):

    def __init__(self, variable_name: str, issue: str) -> None:
        super().__init__(f"The name {variable_name} is not a valid variable name: {issue}")
