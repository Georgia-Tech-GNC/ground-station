#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "sqlite3.h"
#include "mongoose.h"

/**
 * Initializes the web server
 * 
 * @param manager       pointer to the mongoose server
 * @param server_url    the http url that the server should use
 * @param db            the sqlite database that this server should reference
 */
void initialize_web_server(struct mg_mgr *manager, char *server_url, sqlite3* db);

/**
 * Updates the web server by responsding to all API calls
 * 
 * @param manager       pointer to the mongoose server
 * @param timeout_ms    how long this message should wait if no API calls are present.
 */
void update_server(struct mg_mgr *manager, int timeout_ms);

#endif