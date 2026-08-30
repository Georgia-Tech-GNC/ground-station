#include "gs_core.h"
#include "data_helpers.h"
#include "packet_helpers.h"

#include "string.h"


void gs_encode_rocket_kinematics_data(
    const int8_t state,
    const float velocity,
    const float x,
    const float pitch,
    const float roll,
    const float yaw,
    uint8_t* buffer
) {
    memcpy(buffer + 0, &(yaw), 4);
    memcpy(buffer + 4, &(roll), 4);
    memcpy(buffer + 8, &(pitch), 4);
    quantize_float_16(buffer + 12, x, -10, 10);
    quantize_float_16(buffer + 14, velocity, 0, 123.2347934);
    memcpy(buffer + 16, &(state), 1);
}

void gs_decode_rocket_kinematics_data(
    const uint8_t* const buffer,
    gs_rocket_kinematics_data_t* data
) {
    memcpy(&(data->yaw), buffer + 0, 4);
    memcpy(&(data->roll), buffer + 4, 4);
    memcpy(&(data->pitch), buffer + 8, 4);
    unquantize_float_16(&(data->x), buffer + 12, -10, 10);
    unquantize_float_16(&(data->velocity), buffer + 14, 0, 123.2347934);
    memcpy(&(data->state), buffer + 16, 1);
}
