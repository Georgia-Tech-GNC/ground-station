#include "web_server.h"
#include "json_parser.h"

#include <cstdint>
#include <iostream>

namespace web_server {
    mg_mgr mgr;
    HTTPReplyParser database(nullptr);

    thread_local char api_response_buffer[MAX_DATA_PACKET_JSON_STRING_LEN];

    void initialize_web_server(const std::string& address, BufferedSQLDatabase& db) {
        mg_log_set(MG_LL_ERROR); 
        mg_mgr_init(&mgr); 
        mg_http_listen(&mgr, address.c_str(), event_handler, nullptr);
        database.set_db(&db);
    }

    void close_web_server() {
        mg_mgr_free(&mgr);
    }

    void run_web_server_loop(std::atomic_flag& stop_flag) {
        while (!stop_flag.test(std::memory_order_relaxed)) {
            mg_mgr_poll(&mgr, 1000);
        }
    }

    void handle_api_data_response(mg_connection* c, mg_http_message* msg) {
        // Get "idx" parameter from request json
        uint32_t index = static_cast<uint32_t>(mg_json_get_long(msg->body, "$.idx", UINT32_MAX));

        database.get_json_string_by_index(api_response_buffer, MAX_DATA_PACKET_JSON_STRING_LEN, index);
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", api_response_buffer);
    }

    void event_handler(mg_connection* c, int ev, void *ev_data) {
        if (ev == MG_EV_HTTP_MSG) {  // New HTTP request received
            struct mg_http_message* hm = (struct mg_http_message*) ev_data;  

            if (mg_match(hm->uri, mg_str_s("/api/data"), NULL)) {              
                handle_api_data_response(c, hm);

            } else {
                struct mg_http_serve_opts opts = {.root_dir = ".", .fs = &mg_fs_posix};
                mg_http_serve_dir(c, hm, &opts);  // For all other URLs, Serve static files
            }
        }
    }

} // namespace web_server