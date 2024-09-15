#include "mongoose.h"   // To build, run: cc main.c mongoose.c

// HTTP server event handler function
void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_match(hm->uri, mg_str("/api/sum"), NULL)) {
        double num1 = 0.0, num2 = 0.0;
        mg_json_get_num(hm->body, "num1", &num1);  // Extract first number
        mg_json_get_num(hm->body, "num2", &num2);  // Extract second number
        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                    "{%m:%g}\n", MG_ESC("result"), num1 + num2);
    } else {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        struct mg_http_serve_opts opts = { .root_dir = "./web_root/" };
        mg_http_serve_dir(c, hm, &opts);
    }
  }
}

int main(void) {
  struct mg_mgr mgr;  // Declare event manager
  mg_mgr_init(&mgr);  // Initialise event manager
  mg_http_listen(&mgr, "http://0.0.0.0:8080", ev_handler, NULL);  // Setup listener
  for (;;) {          // Run an infinite event loop
    mg_mgr_poll(&mgr, 1000);
  }
  return 0;
}