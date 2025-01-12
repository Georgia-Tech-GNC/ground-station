from abc import ABC

from utils.name_generation import generate_snake_camel_name_pair

class Element(ABC):
    """
        Representation of an element (usually a single datatype)

        :param id:      The numerical id of associated with this unit. An exception will be thrown if the id is 
                        not between 1 and 255, inclusive
        :param name:    The name of this unit. The constructor will check if the name is valid and will raise a 
                        VariableNameInvalidException if it is not.
        :param flags:   Flags that should be associated with this element. Flags are case insensitive
    """

    def __init__(self, name: str, id: int, flags: list[str]):

        if not isinstance(name, str):
            raise TypeError(f"Name was of type {type(name)}, expected string")
        if not isinstance(id, int):
            raise TypeError(f"ID was of type {type(id)}, expected int")
        if not isinstance(flags, list):
            raise TypeError(f"Flags was of type {type(flags)}, expected a list of strings")
        for flag in flags:
            if not isinstance(flag, str):
                raise TypeError(f"Found flag of type {type(flag)}, expected string")
        if id <= 0 or id > 255:
            raise ValueError(f"ID was {id}, expected value in range 1-255, inclusive")
    
        self.__snake_case_name, self.__camel_case_name = generate_snake_camel_name_pair(name)

        self.__flags = map(str.upper, flags)

    @property
    def snake_case_name(self) -> str:
        return self.__snake_case_name
    
    @property
    def camel_case_name(self) -> str:
        return self.__camel_case_name
    
    @property
    def flags(self) -> list[str]:
        return self.__flags
    
    def is_flag(self, flag: str) -> bool:
        if not isinstance(flag, str):
            raise TypeError(f"Inputted flag was of type {type(flag)}, expected string")
        
        return flag.upper() in self.flags

