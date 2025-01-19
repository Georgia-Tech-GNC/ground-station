from jinja2 import Environment, FileSystemLoader
from parser import packets

sql_generate_table_commands = [f"CREATE TABLE {name.title_case} ({", ".join([
    f"{param_name.title_case} {param_type.sqlite}" for param_name, param_type in content.items()
])});" for name, content in packets.items()]

sql_generate_prepared_statement_commands = [f"INSERT INTO {name.title_case} VALUES ({", ".join([
    "?" for _ in content
])});" for name, content in packets.items()]
sql_generate_prepared_statement_commands = {x: len(x) + 1 for x in sql_generate_prepared_statement_commands}

print(sql_generate_table_commands)
print(sql_generate_prepared_statement_commands)

environment = Environment(loader=FileSystemLoader("../templates/"))
template = environment.get_template("sql_executer_c.jinja")
template2 = environment.get_template("sql_executer_h.jinja")

with open('../templates/sql_task_queue.c', 'w+') as f:
    f.write(template.render({'DATA_PACKETS': packets, 'NUM_DATA_PACKET_TYPES': len(sql_generate_table_commands), 'sql_prepared_statement_commands': sql_generate_prepared_statement_commands}))

with open('../templates/sql_task_queue.h', 'w+') as f:
    f.write(template2.render({'DATA_PACKETS': packets, 'NUM_DATA_PACKET_TYPES': len(sql_generate_table_commands), 'sql_prepared_statement_commands': sql_generate_prepared_statement_commands}))
