#include "mongoose.h"   // To build, run: cc main.c mongoose.c
#include <stdio.h>
#include <string.h>


/**
 * Handles incoming requests to the web server
 */
void event_handler(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    //fflush(stdout);
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_match(hm->uri, mg_str("/api/sum"), NULL)) {
        double num1 = 5.0, num2 = 0.0;
        mg_json_get_num(hm->body, "$.vala", &num1);  // Extract first number
        mg_json_get_num(hm->body, "$.valb", &num2);  // Extract second number
        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                    "{%m:%g}\n", MG_ESC("result"), num1 + num2);
    } else {
        printf("failed\n");
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        struct mg_http_serve_opts opts = { .root_dir = "./web_root/" };
        mg_http_serve_dir(c, hm, &opts);
    }
  }
}

void initialize_web_server(struct mg_mgr *manager, char *server_url) {
    mg_mgr_init(manager);  // Initialise event manager
    mg_http_listen(manager, server_url, event_handler, NULL);  // Setup listener
}

void update_server(struct mg_mgr *manager, int timeout_ms) {
    mg_mgr_poll(manager, timeout_ms);
}

