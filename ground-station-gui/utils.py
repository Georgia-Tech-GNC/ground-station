import math
import numpy as np

RAD_2_DEG = 180/math.pi


# {w, x, y, z} -> {x, y, z}
def quaternion_to_euler(quat: np.ndarray[np.float32]) -> np.ndarray[np.float32]:
    t0 = +2.0 * (quat[0] * quat[1] + quat[2] * quat[3])
    t1 = +1.0 - 2.0 * (quat[1] * quat[1] + quat[2] * quat[2])
    roll_x = math.atan2(t0, t1)
    
    t2 = +2.0 * (quat[0] * quat[2] - quat[3] * quat[1])
    t2 = +1.0 if t2 > +1.0 else t2
    t2 = -1.0 if t2 < -1.0 else t2
    pitch_y = math.asin(t2)
    
    t3 = +2.0 * (quat[0] * quat[3] + quat[1] * quat[2])
    t4 = +1.0 - 2.0 * (quat[2] * quat[2] + quat[3] * quat[3])
    yaw_z = math.atan2(t3, t4)
    
    return np.array([roll_x, pitch_y, yaw_z], dtype=np.float32) * RAD_2_DEG


# {w, x, y, z}
def multiply_quaternion(quat1: np.ndarray[np.float32], quat2: np.ndarray[np.float32]) -> np.ndarray[np.float32]:
    w0, x0, y0, z0 = quat1
    w1, x1, y1, z1 = quat2
    return np.array([-x1 * x0 - y1 * y0 - z1 * z0 + w1 * w0,
                     x1 * w0 + y1 * z0 - z1 * y0 + w1 * x0,
                     -x1 * z0 + y1 * w0 + z1 * x0 + w1 * y0,
                     x1 * y0 - y1 * x0 + z1 * w0 + w1 * z0], dtype=np.float32)
