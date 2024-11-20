from utils import verification


class BaseSendableUnit:

    def __init__(self, id: int, name: str):
        """
        Representation of a "thing" 

        :param id:      The numerical id of associated with this unit. An exception will be thrown if the id is 
                        not between 1 and 255, inclusive
        :param name:    The name of this unit. The constructor will check if the name is valid and will raise a 
                        VariableNameInvalidException if it is not.
        """

        if not isinstance(name, str):
            raise TypeError(f"Name was of type {type(name)}, expected string")
        if not isinstance(id, int):
            raise TypeError(f"ID was of type {type(id)}, expected int")
        if id <= 0 or id > 255:
            raise ValueError(f"ID was {id}, expected value in range 1-255, inclusive")

        self.__id = id
        self.__name = name.strip()
        self.__snake_case = verification.generate_valid_snake_variable_name(self.__name)
        self.__title_camel_case = verification.generate_valid_camel_variable_name(self.__name)

    @property
    def id(self) -> int:
        return self.__id
    
    @property
    def name(self) -> str:
        return self.__name
    
    @property
    def name_snake_case(self) -> str:
        return self.__snake_case
    
    @property
    def name_camel_case(self) -> str:
        return self.__title_camel_case
        
    def __str__(self):
        return f"{self.id}: {self.name}"
    
    def __repr__(self):
        return f"Name: {self.name}\n\tID: {self.id}\n\tSnake Case name: {self.name_snake_case}\n\tCamel Case Name: {self.name_camel_case}"