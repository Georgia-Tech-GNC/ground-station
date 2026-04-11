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

def to_nparr(*args):
    """Utility method that returns a numpy array with the given parameters"""
    return np.array(args)

PYRO_INT_TO_VOLT_16_BIT = 3.3 / (2**16)
PYRO_INT_TO_VOLT_12_BIT = 3.3 / (2**12)


TIME_CONVERSION_FACTOR = 1000


def ROCKET_INT_TO_STATE(x: int):
    GROUND = 0, 
    ARMED = 1, 
    FAST_ASCENT = 2, 
    CONTROLLED_ASCENT = 3, 
    PYRO_DEPLOY = 4, 
    STASIS = 5, 
    SD_FLASH = 6, 
    UNCONTROLLED_ASCENT = 7
    if x == 0:
        return "GROUND"
    elif x == 1:
        return "ARMED"
    elif x == 2:
        return "FAST ASCENT"
    elif x == 3:
        return "CONTROLLED ASCENT"
    elif x == 4:
        return "PYRO DEPLOYED"
    elif x == 5:
        return "STASIS"
    elif x == 6:
        return "SD FLASH"
    elif x == 7:
        return "UNCONTROLLED ASCENT"
    else:
        return f"State {x}"



DATA_WINDOWS: dict[str, tuple[
    Callable[[], DataViewerWindow],
    str,
    Callable[[DataViewerWindow], Callable[..., None]],
    list[str]
]] = {
    'Rocket State': (
         lambda: DataViewerWindow(
            ['orientation', 'state', 'servo 1', 'servo 2'],
            [DataTypes.QUATERNION, DataTypes.STRING, DataTypes.FLOAT, DataTypes.FLOAT]
        ),
        'updaterRS',
        lambda window_ref:
        (lambda ww, wx, wy, wz, state, s1, s2: window_ref.set_all_values(
            [to_nparr(ww, wx, wy, wz), ROCKET_INT_TO_STATE(state), s1, s2])),
        ["orientation_w", "orientation_x", "orientation_y",
         "orientation_z", "rocket_state", "servo_cmd_1", "servo_cmd_2"],
    ),
}

GRAPHABLE_VALUES: dict[str, tuple[str, str]] = {
   
}

BUTTON_VALUES: dict[str | tuple[str, bool], Callable] = {
    "PING ROCKET": lambda: send_cmd_with_id(1),
    ("ARM ROCKET (!)", True): lambda: send_cmd_with_id(2),
    ("FIRE PYRO (!)", True): lambda: send_cmd_with_id(3),
    ("ESTOP (!)", True): lambda: send_cmd_with_id(4),
    ("Flash SD Card (!)", True): lambda: send_cmd_with_id(5),
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
                print(f"Caught exception {str(e)}")
                print("Saving the current layout will instead create a new save file")
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

        with dpg.window(tag=dpg_id, label=f"Graph of {', '.join([i[0] for i in graph_entries])}",
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