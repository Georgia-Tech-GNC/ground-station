#ifndef GROUND_STATION_DATABASE_WRITER_H
#define GROUND_STATION_DATABASE_WRITER_H

#include "packet_types.h"

bool initialize_db_writer(const char* db_filepath);

void db_add_data(packet_type_t packet_type, const void* data);

#endif //GROUND_STATION_DATABASE_WRITER_H
