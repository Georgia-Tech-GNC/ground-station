#ifndef GROUND_STATION_ROCKETLIB
#define GROUND_STATION_ROCKETLIB

#ifdef __cplusplus 
extern "C" {
#endif

#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

#define GSRL_NUM_COMMANDS 2
#define GSRL_CMD_PACKET_ID 1
#define GSRL_CMD_PACKET_ACK_ID 2
#define GSRL_RECV_BUFFER_LEN 6

#ifndef GSRL_SEND_BUFFER_LEN
    /** The length of the send buffer used to pending rocket-to-ground messages in bytes */
    #define GSRL_SEND_BUFFER_LEN 256
#endif

enum GroundStationCommandType {
    GSRL_CMD_FIRE_PYRO = 1,
    GSRL_CMD_RUN_TEST = 2,
    GSRL_CMD_OOM_ERROR = 3,
    GSRL_CMD_EMPTY = 4
};

struct GroundStationLink {
    uint8_t send_buffer[GSRL_SEND_BUFFER_LEN];
    size_t current_send_buffer_size;

    uint8_t recv_buffer[GSRL_RECV_BUFFER_LEN];
    size_t  current_recv_buffer_size;

    uint8_t last_seen_cmd_uuids[GSRL_NUM_COMMANDS];
};

/** Flushes any pending bytes in the GroundStationLink's send buffer to an external buffer.
 * Bytes in this buffer should be sent to the ground station.
 * 
 * @param gsl   The GroundStationLink object
 * @param external_buffer       The character buffer that these bytes should be copied to
 * @param external_buffer_size  The size of external_buffer
 * @return      The number of bytes copied.
 */
size_t flush_send_buffer(struct GroundStationLink* gsl, uint8_t* external_buffer, size_t external_buffer_size);

/** Initializes a new GroundStationLink object */
void initialize_ground_station_link(struct GroundStationLink* gsl);

/** Process the incoming byte and add it to a buffer.
 * If a complete command has been parsed, the command type will be returned and 
 * an acknowledgement packet will be added to the GroundStationLink's send buffer
 * 
 * @param gsl                       A GroundStationLink object
 * @param inputted_byte             The recieved byte
 * @return                          The new buffer size. If the value is negative, then the
 *                                  complete message has been recieved (the length is the absolute value of the returned value)
 *                                  
 * Note that the return type GSRL_CMD_OOM_ERROR indicates that the byte was not parsed due to the ground station's recieve buffer being full.
 * If GSRL_CMD_EMPTY is returned, then no command was recieved and no action is required.
 */
enum GroundStationCommandType process_incoming_byte(struct GroundStationLink* gsl, const uint8_t inputted_byte);

/** Adds a data packet of type  to the GroundStationLink's send buffer
 * 
 * @param gsl   The GroundStationLink object
 * @param x Value of parameter x from RocketKinematicsData
 * @param y Value of parameter y from RocketKinematicsData
 * @param z Value of parameter z from RocketKinematicsData
 * @param roll Value of parameter roll from RocketKinematicsData
 * @param pitch Value of parameter pitch from RocketKinematicsData
 * @return true if the operation was successful, and false if the GroundStationLink's send buffer cannot fit this entry.
 */
bool send_rocket_kinematics_data(
    struct GroundStationLink* gsl,
    float x, 
	float y, 
	float z, 
	uint16_t roll, 
	uint16_t pitch
);

/** Adds a data packet of type  to the GroundStationLink's send buffer
 * 
 * @param gsl   The GroundStationLink object
 * @param gps_x Value of parameter gps_x from RocketSensorData
 * @param gps_y Value of parameter gps_y from RocketSensorData
 * @param gps_z Value of parameter gps_z from RocketSensorData
 * @return true if the operation was successful, and false if the GroundStationLink's send buffer cannot fit this entry.
 */
bool send_rocket_sensor_data(
    struct GroundStationLink* gsl,
    int64_t gps_x, 
	int32_t gps_y, 
	int32_t gps_z
);

#ifdef __cplusplus 
} // extern "C"
#endif

#endif