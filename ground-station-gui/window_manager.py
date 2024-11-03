from typing import Callable, Iterable
from gui_templates.data_viewer_window import DataViewerWindow, DataTypes
from gui_templates.RocketCommandsWindow import RocketCommands
from gui_templates.graph_viewer_window import GraphViewerWindow
from gui_templates.logger_window import LoggerWindow
from gui_templates.map_viewer_window import MapViewerWindow
from query_server.data_updater import allDataUpdaters, send_cmd_with_id
import dearpygui.dearpygui as dpg
import numpy as np
import enum
import pickle
import requests
from test_update import set_ground, set_armed, fire_pyro

def to_nparr(*args):
    """Utility method that returns a numpy array with the given parameters"""
    return np.array(args)


def ROCKET_INT_TO_STATE(x: int):
    # TODO: what is the state?
    if x == 1:
        return "IDLE"
    elif x == 2:
        return "GROUND"
    elif x == 3:
        return "ARMED"
    elif x == 4:
        return "FIRED PYRO"
    else:
        return f"State {x}"


def ROCKET_INT_TO_ACTIVE_STATUS(x: int):
    if x == 0:
        return "Disabled"
    elif x == 1:
        return "Active"
    else:
        return f"Unknown value {x}"


DATA_WINDOWS: dict[str, tuple[
    Callable[[], DataViewerWindow],
    str,
    Callable[[DataViewerWindow], Callable[..., None]],
    list[str]
]] = {
    'Rocket Kinematics': (
        lambda: DataViewerWindow(
            ['velocity', 'angular velocity', 'attitude', 'position'],
            [DataTypes.VECTOR, DataTypes.VECTOR, DataTypes.QUATERNION, DataTypes.VECTOR]
        ),
        'updaterRSV',
        lambda window_ref:
        (lambda vx, vy, vz, avx, avy, avz, aw, ax, ay, az, px, py, pz: window_ref.set_all_values(
            [to_nparr(vx, vy, vz), to_nparr(avx, avy, avz), to_nparr(aw, ax, ay, az), to_nparr(px, py, pz)])),
        ["velocity_x", "velocity_y", "velocity_z", "angular_velocity_x", "angular_velocity_y", "angular_velocity_z",
         "attitude_w", "attitude_x", "attitude_y", "attitude_z", "position_x", "position_y", "position_z"],
    ),

    'Rocket State': (
        lambda: DataViewerWindow(
            ['State', 'Firing Channel 1', 'Firing Channel 2', 'Firing Channel 3', 'Firing Channel 4'],
            [DataTypes.STRING, DataTypes.STRING, DataTypes.STRING, DataTypes.STRING, DataTypes.STRING]
        ),
        'updaterRS',
        lambda window_ref:
        (lambda s, fc1, fc2, fc3, fc4: window_ref.set_all_values(
            [ROCKET_INT_TO_STATE(s), ROCKET_INT_TO_ACTIVE_STATUS(fc1), ROCKET_INT_TO_ACTIVE_STATUS(fc2),
             ROCKET_INT_TO_ACTIVE_STATUS(fc3), ROCKET_INT_TO_ACTIVE_STATUS(fc4)])),
        ["rocket_state", "firing_channel_1", "firing_channel_2", "firing_channel_3", "firing_channel_4"]
    ),

    'Rocket Grouund EKF': (
        lambda: DataViewerWindow(
            [f'PN Matrix D{i+1}' for i in range(6)],
            [DataTypes.FLOAT for _ in range(6)]
        ),
        'updaterRGEKF',
        lambda window_ref:
            (lambda d1, d2, d3, d4, d5, d6: window_ref.set_all_values(
                [d1, d2, d3, d4, d5, d6]
            )),
        [f"pn_matrix_d{i+1}" for i in range(6)]
    ), 

    'Rocket Sensor Data': (
        lambda: DataViewerWindow(
            ['Accelerometer', 'Gyro', 'GPS', 'Barometer'],
            [DataTypes.VECTOR, DataTypes.VECTOR, DataTypes.VECTOR, DataTypes.FLOAT]
        ),
        'updaterRSData',
        lambda window_ref:
            (lambda ax, ay, az, gx, gy, gz, gpx, gpy, gpz, b: window_ref.set_all_values(
                [to_nparr(ax, ay, az), to_nparr(gx, gy, gz), to_nparr(gpx, gpy, gpz), b]
            )),
        ["accelerometer_x", "accelerometer_y", "accelerometer_z", "gyro_x", "gyro_y", "gyro_z", "gps_x", "gps_y", "gps_z", "barometer"]
    ),

    'Rocket Servo Deflection': (
        lambda: DataViewerWindow(
            [f'Servo {i+1} deflection' for i in range(4)],
            [DataTypes.FLOAT for _ in range(4)]
        ),
        'updaterRSDef',
        lambda window_ref:
            (lambda d1, d2, d3, d4: window_ref.set_all_values(
                [d1, d2, d3, d4]
            )),
        [f"servo_deflection_{i+1}" for i in range(4)]
    )
}

GRAPHABLE_VALUES: dict[str, tuple[str, str]] = {
    "Rocket State Vector": (None, None),
    "X Velocity": ('updaterRSV', "velocity_x"),
    "Y Velocity": ('updaterRSV', "velocity_y"),
    "Z Velocity": ('updaterRSV', "velocity_z"),
    "Angular X Velocity": ('updaterRSV', "angular_velocity_x"),
    "Angular Y Velocity": ('updaterRSV', "angular_velocity_y"),
    "Angular Z Velocity": ('updaterRSV', "angular_velocity_z"),

    "Servo Deflection": (None, None),
    'Servo 1 Deflection': ('updaterRSDef', 'servo_deflection_1'),
    'Servo 2 Deflection': ('updaterRSDef', 'servo_deflection_2'),
    'Servo 3 Deflection': ('updaterRSDef', 'servo_deflection_3'),
    'Servo 4 Deflection': ('updaterRSDef', 'servo_deflection_4'),

    "Sensor Data": (None, None),
    'Accelerometer X': ('updaterRSData', 'accelerometer_x'),
    'Accelerometer Y': ('updaterRSData', 'accelerometer_y'),
    'Accelerometer Z': ('updaterRSData', 'accelerometer_z'),
    'Gyro X': ('updaterRSData', 'gyro_x'),
    'Gyro Y': ('updaterRSData', 'gyro_y'),
    'Gyro Z': ('updaterRSData', 'gyro_z'),
    'GPS Y': ('updaterRSData', 'gps_y'),
    'Barometer': ('updaterRSData', 'barometer')
}

BUTTON_VALUES: dict[str | tuple[str, bool], Callable] = {
    "IDLE TO GROUND": lambda: send_cmd_with_id(1),
    "GROUND TO ARMED": lambda: send_cmd_with_id(2),
    ("FIRE PYRO (!)", True): lambda: send_cmd_with_id(3),
    "FLASH SD CARD": lambda: send_cmd_with_id(4),
    "RUN VANE ACTUATION TEST": lambda: send_cmd_with_id(5)
}


def run_all(*args: Callable):
    for func in args:
        func()


class WindowType(enum.Enum):
    GRAPH = 0
    DATA = 1
    COMMAND = 2
    LOGGER = 3
    MAP = 4


class WindowManager:
    """
    General purpose class that can create different windows used by the GUI and keeps track of all open windows
    This class can create a pickle file with the current state of the gui, allowing it to be regenerated.
    """

    def __init__(self, pickle_file: str = None):
        """
        Constructor for the window manager class

        :param pickle_file: Optionally, a string denoting a path to a pickle file
                            that stores a prior state of the window manager. Such
                            files can be generated using the pickle_state method.
                            If this file is nonexistent, then this parameter will be ignored
        """
        self.all_loggers: dict[int | str, LoggerWindow] = {}
        if pickle_file is not None:
            try:
                with open(pickle_file, 'rb') as f:
                    data = dict(pickle.load(f))
                
                self.regen_commands = dict(data['regen_commands'])

                max_set_uuid = max(self.regen_commands.keys())
                while dpg.generate_uuid() < max_set_uuid + 10:
                    pass

                for dpg_id, (regen_command, params) in self.regen_commands.items():
                    print(f"Running {regen_command}")
                    self._process_command(dpg_id, regen_command, params)

                max_uuid = dpg.last_item()
                while dpg.generate_uuid() < max_uuid + 10:
                    pass

                
                    
            except Exception as e:
                print(e)
                self.regen_commands: dict[int | str, tuple[WindowType, tuple]] = {}

        else:
            self.regen_commands: dict[int | str, tuple[WindowType, tuple]] = {}

        self.window_counter = 0

    def _process_command(self, dpg_id: int, regen_command: WindowType, params: Iterable) -> None:
        """
        A private method that is used to recreate a window from a pickle save file

        :param dpg_id:          the id of the recreated window
        :param regen_command:   the type of window
        :param params:          the parameters used to construct that window
        """
        if regen_command == WindowType.GRAPH:
            self.create_graph_viewer_window(*params, preset_id=dpg_id)
        elif regen_command == WindowType.DATA:
            self.create_data_viewer_window(*params, preset_id=dpg_id)
        elif regen_command == WindowType.COMMAND:
            self.create_rocket_commands_window(*params, preset_id=dpg_id)
        elif regen_command == WindowType.LOGGER:
            self.create_logger_window(*params, preset_id=dpg_id)
        elif regen_command == WindowType.MAP:
            self.create_map_viewer_window(*params, preset_id=dpg_id)

    def pickle_state(self, filename: str = None) -> None:
        """
        Saves the current state of the window manager class to a pickle file.

        The path to this file can be inputted into this class's constructor to recreate
        the current window state.

        :param filename:    The filepath where the pickle file should be saved
                            (defaults to window_manager_save.pickle).
        """
        save_dict = {
            'regen_commands': self.regen_commands,
        }
        print(save_dict)

        with open(filename if filename is not None else "window_manager_save.pickle", 'wb+') as f:
            pickle.dump(save_dict, f)

    # noinspection PyMethodMayBeStatic
    def _add_window(self, dpg_id: int | str) -> None:
        """
        Private method that is called whenever a new window is created.

        :param dpg_id: the id of the created window
        """
        print(f"added window with id {dpg_id}")

    def _remove_window(self, dpg_id: int | str) -> None:
        """
        Private method that is called whenever a window is removed.

        :param dpg_id: the id of the removed window
        """
        if dpg_id in self.regen_commands:
            del self.regen_commands[dpg_id]

    def _remove_logger(self, dpg_id: int | str) -> None:
        if dpg_id in self.all_loggers:
            del self.all_loggers[dpg_id]

        self._remove_window(dpg_id)

    def add_log(self, msg: str, priority: int) -> None:
        for dpg_id, logger in self.all_loggers.items():
            logger.log_message(msg, priority)

    def create_map_viewer_window(self, window_name: str, preset_id: int = None) -> None:
        if preset_id is None:
            dpg_id = dpg.generate_uuid()
        else:
            dpg_id = preset_id
        
        self.regen_commands[dpg_id] = (WindowType.MAP, (window_name,))

        with dpg.window(
            label=window_name, tag=dpg_id,
            on_close=lambda *args, dpg_id=dpg_id: self._remove_window(dpg_id)
        ) as f:
            MapViewerWindow()

        self._add_window(dpg_id)

    def create_rocket_commands_window(self, window_name: str, preset_id: int = None) -> None:
        if preset_id is None:
            dpg_id = dpg.generate_uuid()
        else:
            dpg_id = preset_id

        self.regen_commands[dpg_id] = (WindowType.COMMAND, (window_name,))

        with dpg.window(
            label=window_name, tag=dpg_id,
            on_close=lambda *args, dpg_id=dpg_id: self._remove_window(dpg_id)
        ) as f:
            RocketCommands(*zip(*BUTTON_VALUES.items()))
        
        self._add_window(dpg_id)


    def create_logger_window(self, window_name: str, width: int, preset_id: int = None) -> None:
        if preset_id is None:
            dpg_id = dpg.generate_uuid()
        else:
            dpg_id = preset_id

        self.regen_commands[dpg_id] = (WindowType.LOGGER, (window_name, width))

        with dpg.window(
            label=window_name, tag=dpg_id,
            on_close=lambda *args, dpg_id=dpg_id: self._remove_logger(dpg_id),
            width=width
        ) as f:
            l = LoggerWindow(width)
            self.all_loggers[dpg_id] = l
        
        # THESE PLACEHOLDERS CAN GO AWAY ONCE LOGGER UPDATER IS WORKING PROPERLY

        self._add_window(dpg_id)
        

    # noinspection PyUnresolvedReferences,PyUnboundLocalVariable,PyShadowingNames
    def create_data_viewer_window(self, window_name: str, preset_id: int = None) -> None:
        """
        Adds a new data viewer window to the gui

        :param window_name: the name of this window (should be a key in the DATA_WINDOWS dictionary)
        :param preset_id:   Optional, an id to use for this window (an id will be auto-generated if
                            this is left blank)
        """
        if window_name not in DATA_WINDOWS.keys():
            print(f"Unknown window type {window_name}")
            return

        constructor, updater, updater_callback, update_query_params = DATA_WINDOWS[window_name]
        updater = allDataUpdaters[updater]
        if preset_id is None:
            dpg_id = dpg.generate_uuid()
        else:
            dpg_id = preset_id

        self.regen_commands[dpg_id] = (WindowType.DATA, (window_name,))

        with dpg.window(label=window_name, tag=dpg_id,
                        on_close=lambda *args, dpg_id=dpg_id, updater=updater: run_all(
                            lambda: self._remove_window(dpg_id),
                            lambda: updater.remove_callback(dpg_id))
                        ) as f:
            x = constructor()
            updater.add_callback(updater_callback(x), dpg_id, update_query_params)

        self._add_window(dpg_id)

    # noinspection PyUnresolvedReferences,PyUnboundLocalVariable
    def create_graph_viewer_window(self, graph_entries: list[tuple[str, str, str]], preset_id: int = None) -> None:
        """
        Adds a new graph viewer window to the gui

        :param graph_entries: A list of tuples of strings representing each item that should be graphed
                              Each tuple should be formatted as follows:
                              Item 1: name of axis.
                              Item 2: string representing a DataUpdater where this data can be found
                                       (must be a valid key in allDataUpdaters).
                              Item 3: the name of this data in the updater listed in item 2.


        :param preset_id:     Optional, an id to use for this window (an id will be auto-generated if
                              this is left blank)
        """
        if any([not allDataUpdaters[upd].is_valid_data_name(name) for _, upd, name in graph_entries]):
            print(f"Invalid list options: {graph_entries}")

        if preset_id is None:
            dpg_id = dpg.generate_uuid()
        else:
            dpg_id = preset_id

        self.regen_commands[dpg_id] = (WindowType.GRAPH, (graph_entries,))

        with dpg.window(tag=dpg_id, label=f"Graph of {", ".join([i[0] for i in graph_entries])}",
                        on_close=lambda *args, dpg_id=dpg_id, graph_entries=graph_entries: run_all(
                            lambda: self._remove_window(dpg_id),
                            lambda: [allDataUpdaters[upd].remove_callback(str(dpg_id)+f"num_{i}") for i, (_, upd, __) in enumerate(graph_entries)]
                        )
        ) as w:
            x = GraphViewerWindow("", [i[0] for i in graph_entries], "timestamp")
            for i, (_, upd, name) in enumerate(graph_entries):
                allDataUpdaters[upd].add_callback(lambda ts, value, i=i: x.update_graph_data(i, ts, value), str(dpg_id) +
                                                  f"num_{i}", ["timestamp", name])

        self._add_window(dpg_id)