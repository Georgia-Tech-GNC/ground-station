#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ring_buffer.h"
#include "globals.h"
#include "packet_types.h"
#include "web_server.h"

ring_buffer_t to_ws_buffer;

struct timespec delay = {
    .tv_sec = 0,
    .tv_nsec = 330000000L
};

static float next_rng() {
    return (float)rand() / (float)RAND_MAX;
}

int main() {
    printf("hello!\r\n");
    if (!initialize_ring_buffer(&to_ws_buffer, 1024)) {
        fprintf(stderr, "Failed to initialize ring buffer\n");
        return EXIT_FAILURE;
    }

    pthread_t thread1;
    if (pthread_create(&thread1, NULL, web_server_thread, nullptr) != 0) {
        perror("Failed to create thread 1");
        return 1;
    }

    // Temporary random data until we get proper driver for radio board
    unsigned int t = 1;
    void* data;
    while (true) {
        adis_data_packet_t adis_packet = {
            .timestamp = t, .accel_x = next_rng(), .accel_y = next_rng(), .accel_z = next_rng(),
            .omega_x = next_rng(), .omega_y = next_rng(), .omega_z = next_rng()
        };
        data = malloc(sizeof(adis_data_packet_t)); memcpy(data, &adis_packet, sizeof(adis_data_packet_t));
        push_to_buffer(&to_ws_buffer, ADIS_DATA_PACKET, data);

        lsm_data_packet_t lsm_packet = {
            .timestamp = t, .accel_x = next_rng(), .accel_y = next_rng(), .accel_z = next_rng(),
            .omega_x = next_rng(), .omega_y = next_rng(), .omega_z = next_rng()
        };
        data = malloc(sizeof(lsm_data_packet_t)); memcpy(data, &lsm_packet, sizeof(lsm_data_packet_t));
        push_to_buffer(&to_ws_buffer, LSM_DATA_PACKET, data);

        barometer_data_packet_t baro_packet = {.timestamp = t, .pressure = next_rng()};
        data = malloc(sizeof(barometer_data_packet_t)); memcpy(data, &baro_packet, sizeof(barometer_data_packet_t));
        push_to_buffer(&to_ws_buffer, BAROMETER_DATA_PACKET, data);

        controls_data_packet_t controls_packet = {
            .timestamp = t, .altitude = next_rng(),
            .servo1_command = next_rng(), .servo2_command = next_rng(),
            .attitude_w = next_rng(), .attitude_x = next_rng(), .attitude_y = next_rng(), .attitude_z = next_rng(),
            .velocity_x = next_rng(), .velocity_y = next_rng(), .velocity_z = next_rng()
        };
        if (next_rng() < 0.5) { memcpy(controls_packet.state, "STATE A", 8); }
        else { memcpy(controls_packet.state, "STATE B", 8); }
        data = malloc(sizeof(controls_data_packet_t)); memcpy(data, &controls_packet, sizeof(controls_data_packet_t));
        push_to_buffer(&to_ws_buffer, CONTROLS_DATA_PACKET, data);

        flight_computer_data_packet_t flight_packet = {
            .timestamp = t, .board_current = next_rng() * 250.0f, .board_voltage = next_rng() * 3.3f,
            .pyro_inhibit_status = next_rng() < 0.5,
            .pyro1_command = next_rng() < 0.5, .pyro1_status = next_rng() < 0.5,
            .pyro2_command = next_rng() < 0.5, .pyro2_status = next_rng() < 0.5
        };
        data = malloc(sizeof(flight_computer_data_packet_t)); memcpy(data, &flight_packet, sizeof(flight_computer_data_packet_t));
        push_to_buffer(&to_ws_buffer, FLIGHT_COMPUTER_DATA_PACKET, data);

        nanosleep(&delay, NULL);
        t++;
    }
}
