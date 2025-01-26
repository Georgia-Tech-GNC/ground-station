#include "protocol.h"

struct LatestData {
  struct RocketStateVector RocketStateVector;
  struct RocketServoDeflection RocketServoDeflection;
  struct RocketState RocketState;
  struct RocketGroundEKF RocketGroundEKF;
  struct RocketSensorData RocketSensorData;
  struct RocketAnalogFeedbackData RocketAnalogFeedbackData;
};

// Initialize such that data_save and data_get can be called
void data_init();

// Save data from a message to the magic data struct, and make it available to data_get
void data_save(int message_id, uint8_t *data, size_t data_size);

// Get the latest data saved by data_save
struct LatestData *data_get();