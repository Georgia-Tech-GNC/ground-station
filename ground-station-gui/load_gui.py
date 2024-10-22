from query_server.data_recorder import DataRecorder
from gui_templates.DataViewerWindow import DataTypes
from graphing.graph_builder_menu import GraphBuilderMenu
from graphing.graph_manager import GraphManager
from query_server.DataUpdater import *
from typing import Callable
import dearpygui.dearpygui as dpg
import numpy as np

def to_nparr(*args) -> np.ndarray:
    return np.array(args)

rsv_data_recorder = DataRecorder(
    ['velocity', 'angular velocity', 'attitude', 'position'], 
    [DataTypes.VECTOR, DataTypes.VECTOR, DataTypes.QUATERNION, DataTypes.VECTOR]
)

data_recorders: dict[str, DataRecorder] = { # Constructor, Update function
    'Rocket State Viewer': rsv_data_recorder
}

updaterRSV.add_callback(
    lambda vx, vy, vz, avx, avy, avz, aw, ax, ay, az, px, py, pz, time: rsv_data_recorder.add_data(time, [to_nparr(vx, vy, vz), to_nparr(avx, avy, avz), to_nparr(aw, ax, ay, az), to_nparr(px, py, pz)]),
    ["velocity_x", "velocity_y", "velocity_z", "angular_velocity_x", "angular_velocity_y", "angular_velocity_z", "attitude_w", "attitude_x", "attitude_y", "attitude_z", "position_x", "position_y", "position_z", "timestamp"],
)

def extract_ts_and_vector(data, vector_index: int):
    times, values = zip(*data)
    return times, list(zip(*values))[vector_index]

graph_manager = GraphManager({
    "x velocity": lambda: extract_ts_and_vector(rsv_data_recorder.get_data_by_index(0), 0),
    "y velocity": lambda: extract_ts_and_vector(rsv_data_recorder.get_data_by_index(0), 1),
    "z velocity": lambda: extract_ts_and_vector(rsv_data_recorder.get_data_by_index(0), 2)
})


def create_window(name: str):
    if name not in data_recorders:
        print(f"Tried to create a window with invalid name {name}!")
        return
    
    if data_recorders[name].is_window_active(): 
        return # Window already exists
    
    with dpg.window(label=name, on_close=data_recorders[name].close_window_callback):
        data_recorders[name].create_window_if_not_active()

def create_add_data_windows_menu_bar():
    for name in data_recorders.keys():
        dpg.add_menu_item(label=name, callback=lambda: create_window(name))

def create_add_graph_window_menu_bar():
    GraphBuilderMenu(graph_manager.get_names(), graph_manager)