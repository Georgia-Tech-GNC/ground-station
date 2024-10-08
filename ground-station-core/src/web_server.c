#include "mongoose.h"   // To build, run: cc main.c mongoose.c
#include "protocol.h"
#include "database.h"
#include "sqlite3.h"

#include <stdio.h>
#include <string.h>

/**
 * Handles incoming requests to the web server
 */
void event_handler(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    //fflush(stdout);
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    HANDLE_API_INPUT(c->fn_data, hm) 
    else {
      fprintf(stderr, "INVALID API REQUEST RECIEVED: %s", hm->uri.buf);
    }
  }
}

void initialize_web_server(struct mg_mgr *manager, char *server_url, sqlite3* db) {
    mg_mgr_init(manager);  // Initialise event manager
    mg_http_listen(manager, server_url, event_handler, db);  // Setup listener
}

void update_server(struct mg_mgr *manager, int timeout_ms) {
    mg_mgr_poll(manager, timeout_ms);
}

