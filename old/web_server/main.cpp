#include "mongoose.h"
#include <iostream>




namespace WebServer {
    mg_mgr manager;

    static void handle_http_event(struct mg_connection* c, mg_http_message* message) {
        if (mg_match(message->uri, mg_str("/api/hello"), NULL)) {              // REST API call?
            mg_http_reply(c, 200, "", "{%m:%d}\n", MG_ESC("status"), 1);    // Yes. Respond JSON
        } else {
            struct mg_http_serve_opts opts = {.root_dir = ".", .fs = &mg_fs_posix};
            mg_http_serve_dir(c, message, &opts);  // For all other URLs, Serve static files
        }
    }

    static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
        if (ev == MG_EV_HTTP_MSG) {  // New HTTP request received
            handle_http_event(c, (mg_http_message*) ev_data);
        }
    }

    enum LogLevel { // Mirror of mongoose.h's enum with C++ namespace scope
        MG_LL_NONE, MG_LL_ERROR, MG_LL_INFO, MG_LL_DEBUG, MG_LL_VERBOSE
    };

    void initialize(LogLevel log_level, std::string address) {
        mg_mgr_init(&manager);
        mg_http_listen(&manager, address.c_str(), event_handler, NULL);
        mg_log_set((int)log_level);
    }

    void close() {
        mg_mgr_free(&manager);
    }

    void update() {
        mg_mgr_poll(&manager, 0);
    }
};


int main() {
    WebServer::initialize(WebServer::LogLevel::MG_LL_DEBUG, "http://0.0.0.0:8000");
    for (;;) {
        WebServer::update();
    }
}
