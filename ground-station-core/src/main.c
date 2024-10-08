#include "mongoose.h"   // To build, run: cc main.c mongoose.c
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "sqlite3.h"
#include "database.h"
#include "protocol.h"
#include "web_server.h"

struct RocketKinematics rkData;
struct MasterElectric elData;

struct mg_mgr manager;

void test_database() {
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

  parse_data_packet_into_sql(db, ROCKETKINEMATICS_MSG_ID, (uint8_t*)&rkData);
  parse_data_packet_into_sql(db, MASTERELECTRIC_MSG_ID, (uint8_t*)&elData);

  printf("%s\n", ROCKETKINEMATICS_SQL_GET_MOST_RECENT);
  char *data[ROCKETKINEMATICS_NUM_VALUES]; 
	query_sql_with_callback(db, data, ROCKETKINEMATICS_SQL_GET_MOST_RECENT);

  for (int i = 0; i < ROCKETKINEMATICS_NUM_VALUES; i++) {
    printf("%s\n", data[i]);
    free(data[i]);
  }

  sqlite3_close(db);
}

int main(void) {
  struct sqlite3 *db = NULL;
  generate_new_database(&db);

  test_database();

  initialize_web_server(&manager, "http://0.0.0.0:8080", db);

  for (int i = 0; i < 10; i++) {
    update_server(&manager, 1000);

    fprintf(stderr, "Second %d\n", i);
  }

  sqlite3_close(db);
}