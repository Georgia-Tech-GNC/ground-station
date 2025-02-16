#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "stdint.h"

typedef enum {
    EMPTY_LOG_MESSAGE_TYPE = 0,        // Used internally to store the case where there are currently no log messages

    ROCKET_DEBUG_MESSAGE_TYPE,     // Debug message recieved from rocket (parameter: debug message ID)
    ROCKET_DEBUG_ERROR_TYPE,       // Error message recieved from rocket (parameter: debug error ID)

    GROUND_STATION_DEBUG_SQL_TASK_QUEUE_FULL, // The SQL task queue is full - data will not be stored until it has more space
    GROUND_STATION_DEBUG_UNKNOWN_MESSAGE_ID,  // Recieved an unknown message ID (parameter: the recieved message ID)

    GROUND_STATION_DEBUG_ROCKET_KINEMATICS_PACKET_WRONG_SIZE, // Recieved a rocket kinematics packet that was the wrong size (parameter: recieved packet size)
    
    GROUND_STATION_DEBUG_ROCKET_STATE_PACKET_WRONG_SIZE, // Recieved a rocket state packet that was the wrong size (parameter: recieved packet size)
} LogMessageType;

typedef struct {
    LogMessageType log_message;
    uint32_t message_parameter;
} LogMessage;

typedef struct {
    float x_position;
    float y_position;
    float z_position;
    int16_t x_velocity;
    int16_t y_velocity;
    int16_t z_velocity;
    int32_t acceleration_magnitude;
} RocketKinematics;
#define DATA_MSG_ROCKET_KINEMATICS_PACKET_SIZE 22
#define DATA_MSG_ROCKET_KINEMATICS_MSG_ID 10

typedef struct {
    uint8_t state;
    uint8_t firing_channel_1;
    uint8_t firing_channel_2;
    uint8_t firing_channel_3;
} RocketState;
#define DATA_MSG_ROCKET_STATE_PACKET_SIZE 4
#define DATA_MSG_ROCKET_STATE_MSG_ID 11

#endif