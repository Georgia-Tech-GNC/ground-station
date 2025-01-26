#include "stdatomic.h"
#include "atomic.h"

// sequential consistency is enough in this case
volatile int ready_data_index;

struct LatestData latest_data[2];

void data_init() {
  atomic_init(&ready_data_index, 0);
}

void data_save(int message_id, uint8_t *data, size_t data_size) {
  int read_index = atomic_load(&ready_data_index);
  int write_index = (read_index + 1) % 2;
  latest_data[write_index] = latest_data[read_index];
  switch (message_id) {
	case ROCKETSTATEVECTOR_MSG_ID: {
    if (data_size != ROCKETSTATEVECTOR_SIZE) {
      printf("warning: wrong data size!\n");
    }
		struct RocketStateVector cvt_data;
		RocketStateVector_decode(data, &cvt_data);
    latest_data[write_index].RocketStateVector = cvt_data;
    break;
  }
	case ROCKETSERVODEFLECTION_MSG_ID: {
    if (data_size != ROCKETSERVODEFLECTION_SIZE) {
      printf("warning: wrong data size!\n");
    }
		struct RocketServoDeflection cvt_data;
		RocketServoDeflection_decode(data, &cvt_data);
    latest_data[write_index].RocketServoDeflection = cvt_data;
    break;
  }
	case ROCKETSTATE_MSG_ID: {
    if (data_size != ROCKETSTATE_SIZE) {
      printf("warning: wrong data size!\n");
    }
		struct RocketState cvt_data;
		RocketState_decode(data, &cvt_data);
    latest_data[write_index].RocketState = cvt_data;
    break;
  }
	case ROCKETGROUNDEKF_MSG_ID: {
    if (data_size != ROCKETGROUNDEKF_SIZE) {
      printf("warning: wrong data size!\n");
    }
		struct RocketGroundEKF cvt_data;
		RocketGroundEKF_decode(data, &cvt_data);
    latest_data[write_index].RocketGroundEKF = cvt_data;
    break;
  }
	case ROCKETSENSORDATA_MSG_ID: {
    if (data_size != ROCKETSENSORDATA_SIZE) {
      printf("warning: wrong data size!\n");
    }
		struct RocketSensorData cvt_data;
		RocketSensorData_decode(data, &cvt_data);
    latest_data[write_index].RocketSensorData = cvt_data;
    break;
  }
	case ROCKETANALOGFEEDBACKDATA_MSG_ID: {
    if (data_size != ROCKETANALOGFEEDBACKDATA_SIZE) {
      printf("warning: wrong data size!\n");
    }
		struct RocketAnalogFeedbackData cvt_data;
		RocketAnalogFeedbackData_decode(data, &cvt_data);
    latest_data[write_index].RocketAnalogFeedbackData = cvt_data;
    break;
  }
  default:
    break;
  }
  atomic_store(&ready_data_index, (write_index + 1) % 2);
}

struct LatestData *data_get() {
  return &latest_data[atomic_load(&ready_data_index)];
}