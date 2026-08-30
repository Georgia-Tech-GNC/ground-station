#ifndef GS_CORE_H
#define GS_CORE_H

#include "stddef.h"
#include "stdint.h"

enum : size_t {
    GS_ROCKET_KINEMATICS_DATA_PAYLOAD_SIZE = 17
};

typedef struct GS_ROCKET_KINEMATICS {
    int8_t state;
    float velocity;
    float x;
    float pitch;
    float roll;
    float yaw;
} gs_rocket_kinematics_data_t;

void gs_encode_rocket_kinematics_data(
    const int8_t state,
    const float velocity,
    const float x,
    const float pitch,
    const float roll,
    const float yaw,
    uint8_t* buffer
);

void gs_decode_rocket_kinematics_data(
    const uint8_t* const buffer,
    gs_rocket_kinematics_data_t* data
);

#endif