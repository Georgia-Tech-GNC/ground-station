#include "mongoose.h"   // To build, run: cc main.c mongoose.c
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "sqlite3.h"
#include "database.h"
#include "protocol.h"
/*
// HTTP server event handler function
void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    //fflush(stdout);
    printf("hi!\n");
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_match(hm->uri, mg_str("/api/sum"), NULL)) {
        double num1 = 5.0, num2 = 0.0;
        printf("recv\n");
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
*/

struct RocketKinematics rkData;
struct MasterElectric elData;

int main(void) {
  /*struct mg_mgr mgr;  // Declare event manager
  mg_mgr_init(&mgr);  // Initialise event manager
  mg_http_listen(&mgr, "http://0.0.0.0:8080", ev_handler, NULL);  // Setup listener
  for (;;) {          // Run an infinite event loop
    mg_mgr_poll(&mgr, 10);
  }
  return 0;
  */

  struct sqlite3 *db = NULL;

  rkData.attitude_w = 0.1212342341334f;
  rkData.attitude_x = 0.903231248321f;
  rkData.attitude_y = 0.57414145414941f;
  rkData.attitude_z = 1.1245149847248914f;
  rkData.velocity = 49421304;
  rkData.timestamp = time(NULL);


  elData.voltage = 123.4125;
  elData.current = 35.41561;
  elData.electricity_status = 0;
  elData.timestamp = time(NULL);

  generate_new_database(&db);

  parse_data_packet_into_sql(db, 2, (uint8_t*)&rkData);
  parse_data_packet_into_sql(db, 3, (uint8_t*)&elData);

  printf("%s\n", ROCKETKINEMATICS_SQL_GET_MOST_RECENT);
  char *data[ROCKETKINEMATICS_NUM_VALUES]; 
	query_sql_with_callback(db, data, ROCKETKINEMATICS_SQL_GET_MOST_RECENT);

  for (int i = 0; i < ROCKETKINEMATICS_NUM_VALUES; i++) {
    printf("%s\n", data[i]);
    free(data[i]);
  }

  sqlite3_close(db);
}