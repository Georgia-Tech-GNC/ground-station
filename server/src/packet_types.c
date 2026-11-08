#include "packet_types.h"

size_t get_packet_struct_size(packet_type_t packet_type) {
    switch (packet_type) {
        case ADIS_DATA_PACKET: return sizeof(adis_data_packet_t);
        case LSM_DATA_PACKET: return sizeof(lsm_data_packet_t);
        case BAROMETER_DATA_PACKET: return sizeof(BAROMETER_DATA_PACKET);
        case CONTROLS_DATA_PACKET: return sizeof(CONTROLS_DATA_PACKET);
        case FLIGHT_COMPUTER_DATA_PACKET: return sizeof(FLIGHT_COMPUTER_DATA_PACKET);
        default: return 0;
    }
}
