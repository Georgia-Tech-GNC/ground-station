#include "globals.h"
#include "ring_buffer.h"
#include "packet_types.h"

#include <stdio.h>
#include <stdlib.h>

void* web_server_thread(void* arg) {
    printf("Started web server!\n");
    while (true) {
        uint8_t packet_type;
        void* packet = pop_from_buffer(&to_ws_buffer, &packet_type);
        switch (packet_type) {
            case ADIS_DATA_PACKET:
                adis_data_packet_t* adis_data_packet = packet;
                break;
            case LSM_DATA_PACKET:
                lsm_data_packet_t* lsm_data_packet = packet;
                break;
            case BAROMETER_DATA_PACKET:
                barometer_data_packet_t* barometer_data_packet = packet;
                break;
            case CONTROLS_DATA_PACKET:
                controls_data_packet_t* controls_data_packet = packet;
                break;
            case FLIGHT_COMPUTER_DATA_PACKET:
                flight_computer_data_packet_t* flight_computer_data_packet = packet;
                break;
            default:
                // Unknown packet type?
                break;
        }
        printf("Recv packet: %d\n", packet_type);
        free(packet);
    }
}
