from utils import verification
from parse_types import base

class Command(base.BaseSendableUnit):

    def __init__(self, command_id: int, name: str, *flags: str):
        """
        Representation of a command that could be sent to the rocket

        :param command_id: The command id of this command (must be between 1 and 255, inclusive, unchecked)
        :param name:       The name of this command. The constructor will check if the name is valid and will
                           raise a VariableNameInvalidException if it is not.
        :param flags:      Additional flags passed into this method
        """
        super().__init__(command_id, name)

        self.__requires_confirmation = False

        for flag in flags:
            if not isinstance(flag, str):
                raise TypeError(f"Flag was of type {type(flag)}, expected string")
            
            match flag.lower():
                case 'requireconfirmation':
                    self.__requires_confirmation = True
                case _:
                    raise ValueError(f'Unknown flag {flag}, supported flags are "RequireConfirmation"')

    @property
    def require_confirmation(self):
        return self.__requires_confirmation    

    def __str__(self):
        return f"Command " + super().__str__()
    
    def __repr__(self):
        return super().__repr__() + f"\n\tType: Command\n\tRequire Confirmation: {self.require_confirmation}"