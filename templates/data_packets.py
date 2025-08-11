from variables import VariableType

class DataPacket:
    def __init__(self, name: str, variables: dict[str, VariableType]):
        self.title_name = name.title().replace(' ', '')
        self.variable_name = name.lower().replace(' ', '_')
        self.variables = [
            {'name': var_name, 'sqlite_binding': var_type.sqlite_bind, 'length': var_type.size, 'type': var_type.typename} 
            for var_name, var_type in variables.items()
        ]
        self.byteform = {'length': sum([var_type.size for var_type in variables.values()])}
        self.variables_by_type = {}
        for var_name, var_type in variables.items():
            if var_type.typename not in self.variables_by_type: 
                self.variables_by_type[var_type.typename] = []
            self.variables_by_type[var_type.typename].append(var_name)
        self.sqlite_prepared_statement = f"INSERT INTO {self.title_name} VALUES ({", ".join(["?"] * (1 + len(variables)))})"
        self.sql_table_gen_command = f"CREATE TABLE IF NOT EXISTS {self.title_name} (timestamp INTEGER PRIMARY KEY, { \
            ', '.join([f'{var_name} {var_type.sql_type}' for var_name, var_type in variables.items()]) \
        })"