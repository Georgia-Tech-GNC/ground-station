
def generate_get_msg_len_header():
    comment = """/**
 * Calculates the size of a message given its ID
 * @param id	the ID of the message
 * @return		the size of the message in bytes, or -1 if the message id is invalid
 */
"""
    return comment + "int get_msg_size(int message_id);\n\n"

def generate_get_msg_len(data_type_names: list[str]):
    content="int get_msg_size(int message_id) {\n\tswitch (message_id) {\n"

    for name in data_type_names:
        content += f"\t\tcase {name.upper()}_MSG_ID:\n\t\t\treturn {name.upper()}_SIZE;\n"

    return content + "\t\tdefault:\n\t\t\treturn -1;\n\t}\n}\n\n"




def generate_get_is_data_msg_header():
    comment="""/**
 * Returns true if this message id corresponds to a data packet, and false otherwise
 */
"""
    return comment + "bool is_data_send_msg(int message_id);\n\n"

def generate_get_is_data_msg(last_data_msg_id: int):
    header="bool is_data_send_msg(int message_id) {\n"
    return header + f"\treturn (message_id >= 2) && (message_id <= {last_data_msg_id});" + "\n}\n\n"




def generate_cvt_packet_to_sql_header():
    comment = comment = """   /**
	 * Takes a recieved data packet and converts it into an SQL command
	 * 
	 * @param message_id	The id associated with this message
	 * @param data			The data recieved (as an array of bytes)
	 * @param sql_cmd		A char* buffer where the SQL command will be saved (must be at least as big as MAX_SQL_WRITE_CMD_SIZE)
	 * 
	 * @return				true if the operation was successful, and false otherwise.
	 */
"""
    return comment + "\tbool convert_data_msg_to_sql_cmd(int message_id, uint8_t *data, char *sql_cmd);\n\n"

def generate_cvt_packet_to_sql(data_type_names):
    content = """bool convert_data_msg_to_sql_cmd(int message_id, uint8_t *data, char *sql_cmd) {
		if (!is_data_send_msg(message_id)) {
			return false;
		}

		switch (message_id) {
"""

    content += "\t\t\t"
    for name in data_type_names:
        content += f"case {name.upper()}_MSG_ID:" + " {\n"
        content += f"\t\t\t\tstruct {name}* cvt_data = (struct {name}*) data;\n"
        content += "\t\t\t\tchar timestamp[20];\n"
        content += "\t\t\t\tgenerate_sql_timestamp_from_long(cvt_data->timestamp, timestamp);\n"
        content += f"\t\t\t\t{name.upper()}_SQL_ADD_ENTRY(sql_cmd, cvt_data, timestamp);\n\t\t\t\treturn true;\n" + "\t\t\t} "

    content += f"default:\n\t\t\t\treturn false;\n"

    return content + "\t\t}\n\t}\n\n"

def generate_cvt_to_sql_time():
    return """\tvoid generate_sql_timestamp_from_long(time_t time, char* buffer) {
		strftime(buffer, 20, "%Y-%M-%d %H:%M:%S", localtime(&time));
	}\n
    """

def generate_api_if_else(names):
    return " else ".join([
        (
          f"\t\tif (mg_match(hm->uri, mg_str({name.upper()}_API_PATH), NULL))" + " { \\" +
		   "\n\t\t\tchar** data; \\" + 
		  f"\n\t\t\tquery_sql_with_callback(db, &data, {name.upper()}_SQL_GET_MOST_RECENT); \\" + 
		  f"\n\t\t\tmg_http_reply(c, 200, \"Content-Type: application/json\\r\\n\", {name.upper()}_SQL_SELECT_TO_JSON(data)); \\" + 
           "\n\t\t}"
	    ) for name in names
    ])