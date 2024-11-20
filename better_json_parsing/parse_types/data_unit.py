from abc import ABC


class DataUnit(ABC):
    pass

class Boolean:

    def __init__(self, *names: str):
        if len(names) <= 0 or len(names) > 4:
            raise ValueError(f"Cannot encode more then 4 or less then 1 booleans in a single byte (tried to encode {len(values)})")
        
        for name in names:
            if not isinstance(name, str):
                raise TypeError(f"Expected names to be strings (recieved {name})")
            
