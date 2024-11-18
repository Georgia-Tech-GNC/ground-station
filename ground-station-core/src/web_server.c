#include "mongoose.h"   // To build, run: cc main.c mongoose.c
#include "protocol.h"
#include "database.h"
#include "sqlite3.h"
#include "logger.h"
#include "command_scheduler.h"

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
    else if (mg_match(hm->uri, mg_str(COMMAND_API_PATH), NULL)) {
      double cmd_id_double = 0.0;
      mg_json_get_num(hm->body, "$.id", &cmd_id_double);  // Extract first number
      mg_http_reply(c, 200, "", "{%m:%d}\n", MG_ESC("status"), 1);
      send_command(cmd_id_double + 1e-9);
    } else if (mg_match(hm->uri, mg_str("/api/logs/get"), NULL)) {
      char* f;
      double logs_get_param = 0;
      mg_json_get_num(hm->body, "$.msg_cnt", &logs_get_param);  // Extract first number
      query_sql_get_logs(c->fn_data, logs_get_param + 1e-9, &f);
      mg_http_reply(c, 200, "", f);
      free(f);
    } else if (mg_match(hm->uri, mg_str("/api/packets/info"), NULL)) {
      int num_succeeded = get_num_packets_recieved();
      int num_failed = get_num_packets_failed();
      mg_http_reply(c, 200, "", "{%m:%d, %m:%d}\n", MG_ESC("succeeded"), num_succeeded, MG_ESC("failed"), num_failed);
    } else {
      log_warning("INVALID API REQUEST RECIEVED: %s\n", hm->uri.buf);
      mg_http_reply(c, 401, NULL, "{}");
    }
  }
}

bool initialize_web_server(struct mg_mgr *manager, char *server_url, sqlite3* db) {
    mg_mgr_init(manager);  // Initialise event manager
    if (mg_http_listen(manager, server_url, event_handler, db) == NULL) {  // Setup listener
        log_error("Error initializing web server\n");
        return false;
    }
    
    return true;
}


void update_server(struct mg_mgr *manager, int timeout_ms) {
    mg_mgr_poll(manager, timeout_ms);
}

