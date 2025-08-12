#include "json_parser.h"
#include "datatypes.h"

#include "mongoose.h"

void HTTPReplyParser::get_json_string_by_index(char* buffer, size_t len, uint32_t index) {
    if (!db) {
        mg_snprintf(buffer, len, "{}");
        return;
    }
    
    switch (index) {
        case (static_cast<uint32_t>(DataPacketType::ROCKET_KINEMATICS_DATA)): {
            RocketKinematicsData data = db->see_most_recent_entry<RocketKinematicsData>();
            mg_snprintf(
                buffer, len,
                "{\"data\":[%g,%g,%g,%hu,%hu]}",
                data.x, data.y, data.z, data.roll, data.pitch
            );
            break;
        }
        case (static_cast<uint32_t>(DataPacketType::ROCKET_SENSOR_DATA)): {
            RocketSensorData data = db->see_most_recent_entry<RocketSensorData>();
            mg_snprintf(
                buffer, len,
                "{\"data\":[%lld,%d,%d]}",
                data.gps_x, data.gps_y, data.gps_z
            );
            break;
        }
        default:
            mg_snprintf(buffer, len, "{}"); // Return an empty buffer for invalid indices
    }
}