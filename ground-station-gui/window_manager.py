from typing import Callable, Any
from gui_templates.DataViewerWindow import DataViewerWindow, DataTypes
from query_server.DataUpdater import updaterRSV, updaterRS
import dearpygui.dearpygui as dpg
import numpy as np

def to_nparr(*args):
    return np.array(args)

def ROCKET_INT_TO_STATE(x: int):
    # TODO: what is the state?
    return f"State {x}"

def ROCKET_INT_TO_ACTIVE_STATUS(x: int):
    if (x == 0): return "Disabled"
    elif (x == 1): return "Active"
    else: return f"Unknown value {x}"

WINDOWS: dict[str, dict[str, Any]] = {
    'Data Windows': {
        'Rocket Kinematics': (
            lambda: DataViewerWindow(
                ['velocity', 'angular velocity', 'attitude', 'position'], 
                [DataTypes.VECTOR, DataTypes.VECTOR, DataTypes.QUATERNION, DataTypes.VECTOR]
            ),
            updaterRSV,
            lambda window_ref:
                (lambda vx, vy, vz, avx, avy, avz, aw, ax, ay, az, px, py, pz: window_ref.set_all_values([to_nparr(vx, vy, vz), to_nparr(avx, avy, avz), to_nparr(aw, ax, ay, az), to_nparr(px, py, pz)])),
            ["velocity_x", "velocity_y", "velocity_z", "angular_velocity_x", "angular_velocity_y", "angular_velocity_z", "attitude_w", "attitude_x", "attitude_y", "attitude_z", "position_x", "position_y", "position_z"],
        ),

        'Rocket State': (
            lambda: DataViewerWindow(
                ['State', 'Firing Channel 1', 'Firing Channel 2', 'Firing Channel 3', 'Firing Channel 4'], 
                [DataTypes.STRING, DataTypes.STRING, DataTypes.STRING, DataTypes.STRING, DataTypes.STRING]
            ),
            updaterRS,
            lambda window_ref:
                (lambda s, fc1, fc2, fc3, fc4: window_ref.set_all_values([ROCKET_INT_TO_STATE(s), ROCKET_INT_TO_ACTIVE_STATUS(fc1), ROCKET_INT_TO_ACTIVE_STATUS(fc2), ROCKET_INT_TO_ACTIVE_STATUS(fc3), ROCKET_INT_TO_ACTIVE_STATUS(fc4)])),
            ["rocket_state", "firing_channel_1", "firing_channel_2", "firing_channel_3", "firing_channel_4"]
        )
    }
}

class WindowManager:

    def __init__(self, windows: dict[int|str, Callable] = None):
        if windows is None:
            self.windows: dict[int|str, Callable] = {}
        else: self.windows = windows

    def _add_window(self, id: int|str, constructor: Callable):
        self.windows[id] = constructor

    def _remove_window(self, id: int|str):
        if id in self.windows:
            del self.windows[id]

    def construct_all_windows(self):
        for _, constructor in self.windows:
            constructor()

    def create_data_viewer_window(self, window_name: str):
        if window_name not in WINDOWS['Data Windows'].keys():
            print(f"Unknown window type {window_name}")
            return
        
        constructor, updater, updater_callback, update_query_params = WINDOWS['Data Windows'][window_name]
        id = dpg.generate_uuid()

        # TODO: remove updater link on close
        with dpg.window(label=window_name, on_close=lambda: self._remove_window(id)):
            x = constructor()
            updater.add_callback(updater_callback(x), update_query_params)

        self._add_window(id, constructor)

