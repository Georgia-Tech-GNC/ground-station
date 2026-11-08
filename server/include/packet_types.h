#ifndef GROUND_STATION_PACKET_TYPES_H
#define GROUND_STATION_PACKET_TYPES_H
#include <stddef.h>

typedef enum PacketType {
    ADIS_DATA_PACKET                = 0,
    LSM_DATA_PACKET                 = 1,
    BAROMETER_DATA_PACKET           = 2,
    CONTROLS_DATA_PACKET            = 3,
    FLIGHT_COMPUTER_DATA_PACKET     = 4
} packet_type_t;

size_t get_packet_struct_size(packet_type_t packet_type);

typedef struct ADISDataPacket {
    float omega_x, omega_y, omega_z;
    float accel_x, accel_y, accel_z;
    unsigned int timestamp;
} adis_data_packet_t;

typedef struct LSMDataPacket {
    float omega_x, omega_y, omega_z;
    float accel_x, accel_y, accel_z;
    unsigned int timestamp;
} lsm_data_packet_t;

typedef struct BarometerDataPacket {
    float pressure;
    unsigned int timestamp;
} barometer_data_packet_t;

typedef struct ControlsDataPacket {
    char state[16];
    float servo1_command, servo2_command;
    float attitude_w, attitude_x, attitude_y, attitude_z;
    float velocity_x, velocity_y, velocity_z;
    float altitude;
    unsigned int timestamp;
} controls_data_packet_t;

typedef struct FlightComputerDataPacket {
    bool pyro_inhibit_status;
    bool pyro1_command, pyro1_status;
    bool pyro2_command, pyro2_status;
    float board_voltage, board_current;
    unsigned int timestamp;
} flight_computer_data_packet_t;

#endif //GROUND_STATION_PACKET_TYPES_H
