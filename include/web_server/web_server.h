#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "mongoose.h"
#include "buffered_db.h"

#include <string>
#include <atomic>

namespace web_server {

    void event_handler(mg_connection *c, int ev, void *ev_data);

    void initialize_web_server(const std::string& address, BufferedSQLDatabase& database);

    void close_web_server();

    void run_web_server_loop(std::atomic_flag& stop_flag);
};


#endif