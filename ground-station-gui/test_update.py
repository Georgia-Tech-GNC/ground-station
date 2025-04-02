from query_server.data_updater import updaterRSV, updaterRSDef, updaterRS, updaterRGEKF, updaterRSData
import random
import math

ROCKET_STATE = 1

def set_ground():
    global ROCKET_STATE
    ROCKET_STATE = 2

def set_armed():
    global ROCKET_STATE
    ROCKET_STATE = 3

def fire_pyro():
    global ROCKET_STATE
    ROCKET_STATE = 4

def noise(scale: int = 1000):
    return random.randint(1, scale)/1500

LOG_OPTIONS = [
    ('Rocket status update recieved', 2),
    ('Logging module connection temporarily dropped', 2),
    ('Battery current draw spike', 3),
    ('Ground EKF verification running', 2),
    ("Concurrent race viewer detected", 2),
    ("Hash verification success", 2)
]

def create_random_log_message():
    return LOG_OPTIONS[random.randint(0, len(LOG_OPTIONS)-1)]

def update_all_tests(time: float) -> None:
    global ROCKET_STATE

    angle = time/5 + noise(10)
    if time > 5: return 

    updaterRSV.test({
            "velocity_x": time * time * 5.0 + noise(10000),
            "velocity_y": 5 + noise(500),
            "velocity_z": time + noise(1000),
            "angular_velocity_x": time - 3 + noise(),
            "angular_velocity_y": time * 4 + noise(),
            "angular_velocity_z": time * time + noise(),
            "attitude_w": math.cos(angle),
            "attitude_x": math.sin(angle),
            "attitude_y": 0,
            "attitude_z": 0,
            "position_x": time / 2 + noise(5000),
            "position_y": time/2 + noise(1000),
            "position_z": time + noise(20),
            "timestamp": time * 1000
        })
    
    updaterRSDef.test({
        "servo_deflection_1": 25*math.cos(angle*angle) + noise(250),
        "servo_deflection_2": 30*math.sin(angle - 50) + noise(750),
        "servo_deflection_3": 15*math.cos(angle - 15) - 6*math.sin(angle) + noise(500),
        "servo_deflection_4": 27*math.sin(angle*30 - 2) + noise(500),
        "timestamp": time * 100
    })

    updaterRS.test({
        "rocket_state": ROCKET_STATE,
        "firing_channel_1": 0 if ROCKET_STATE < 2 else 1,
        "firing_channel_2": 0 if ROCKET_STATE < 2 else 1,
        "firing_channel_3": 0 if ROCKET_STATE < 3 else 1,
        "firing_channel_4": 0 if ROCKET_STATE < 3 else 1,
        "timestamp": time * 1000
    })

    updaterRGEKF.test({
        "pn_matrix_d1": noise(10),
        "pn_matrix_d2": noise(10),
        "pn_matrix_d3": noise(10),
        "pn_matrix_d4": noise(10),
        "pn_matrix_d5": noise(10),
        "pn_matrix_d6": noise(10),
        "timestamp": time*1000
    })

    updaterRSData.test({
        "accelerometer_x": 30 + noise(500),
        "accelerometer_y": 24 + noise(5000),
        "accelerometer_z": 12 + time + noise(500),
        "gyro_x": 12 + noise(2000),
        "gyro_y": 9.8 + noise(2000),
        "gyro_z": time + noise(2000),
        "gps_x": time*time + noise(2000),
        "gps_y": time/5 + noise(500),
        "gps_z": time - noise(2000),
        "barometer": time/5 + noise(500),
        "accel_bias_x": 0,
        "accel_bias_y": 0,
        "accel_bias_z": 0,
        "gyro_bias_x": 0,
        "gyro_bias_y": 0,
        "gyro_bias_z": 0,
        "gps_offset_x": 0,
        "gps_offset_y": 0,
        "gps_offset_z": 0,
        "barometer_offset": 0,
        "timestamp": time*1000
    })


    
    