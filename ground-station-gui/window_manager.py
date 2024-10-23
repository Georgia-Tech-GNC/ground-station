from typing import Callable
from gui_templates.data_viewer_window import DataViewerWindow, DataTypes
from gui_templates.graph_viewer_window import GraphViewerWindow
from query_server.data_updater import allDataUpdaters
import dearpygui.dearpygui as dpg
import numpy as np
import enum
import pickle


def to_nparr(*args):
    return np.array(args)


def ROCKET_INT_TO_STATE(x: int):
    # TODO: what is the state?
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
    )
}

GRAPHABLE_VALUES: dict[str, tuple[str, str]] = {
    "X Velocity": ('updaterRSV', "velocity_x"),
    "Y Velocity": ('updaterRSV', "velocity_x"),
    "Z Velocity": ('updaterRSV', "velocity_x"),
    "Angular X Velocity": ('updaterRSV', "angular_velocity_x"),
    "Angular Y Velocity": ('updaterRSV', "angular_velocity_y"),
    "Angular Z Velocity": ('updaterRSV', "angular_velocity_z"),
}


def run_all(*args: Callable):
    for func in args:
        func()


class WindowType(enum.Enum):
    GRAPH = 0
    DATA = 1


class WindowManager:

    def __init__(self, pickle_file: str = None):
        if pickle_file is not None:
            with open(pickle_file, 'rb') as f:
                data = dict(pickle.load(f))

            self.data_window_counter = int(data['data_window_counter'])
            self.graph_window_counter = int(data['graph_window_counter'])
            self.regen_commands = dict(data['regen_commands'])

            for dpg_id, (regen_command, params) in self.regen_commands.items():
                self._process_command(dpg_id, regen_command, params)

        else:
            self.regen_commands: dict[int | str, tuple[WindowType, tuple]] = {}

            self.data_window_counter = 0
            self.graph_window_counter = 0

    def _process_command(self, dpg_id, regen_command, params):
        if regen_command == WindowType.GRAPH:
            self.create_graph_viewer_window(*params, preset_id=dpg_id)
        elif regen_command == WindowType.DATA:
            self.create_data_viewer_window(*params, preset_id=dpg_id)

    def pickle_state(self, filename: str = None):
        save_dict = {
            'regen_commands': self.regen_commands,
            'data_window_counter': self.data_window_counter,
            'graph_window_counter': self.graph_window_counter
        }

        with open(filename if filename is not None else "window_manager_save.pickle", 'wb+') as f:
            pickle.dump(save_dict, f)

    # noinspection PyMethodMayBeStatic
    def _add_window(self, dpg_id: int | str):
        print(f"added window with id {dpg_id}")

    def _remove_window(self, dpg_id: int | str):
        if dpg_id in self.regen_commands:
            del self.regen_commands[dpg_id]

    # noinspection PyUnresolvedReferences,PyUnboundLocalVariable,PyShadowingNames
    def create_data_viewer_window(self, window_name: str, preset_id: int | str = None):
        if window_name not in DATA_WINDOWS.keys():
            print(f"Unknown window type {window_name}")
            return

        constructor, updater, updater_callback, update_query_params = DATA_WINDOWS[window_name]
        updater = allDataUpdaters[updater]
        if preset_id is None:
            dpg_id = f"DATA_WINDOW_CONTAINER_NUM_{self.data_window_counter}"
        else:
            dpg_id = preset_id

        self.data_window_counter += 1
        self.regen_commands[dpg_id] = (WindowType.DATA, (window_name,))

        with dpg.window(label=window_name,
                        on_close=lambda *args, dpg_id=dpg_id, updater=updater: run_all(
                            lambda: self._remove_window(dpg_id),
                            lambda: updater.remove_callback(dpg_id))
                        ):
            x = constructor()
            updater.add_callback(updater_callback(x), dpg_id, update_query_params)

        self._add_window(dpg_id)

    # noinspection PyUnresolvedReferences,PyUnboundLocalVariable
    def create_graph_viewer_window(self, graph_entries: list[tuple[str, str, str]], preset_id: int | str = None):
        if any([not allDataUpdaters[upd].is_valid_data_name(name) for _, upd, name in graph_entries]):
            print(f"Invalid list options: {graph_entries}")

        if preset_id is None:
            dpg_id = f"GRAPH_WINDOW_CONTAINER_NUM_{self.graph_window_counter}"
        else:
            dpg_id = preset_id

        self.graph_window_counter += 1
        self.regen_commands[dpg_id] = (WindowType.GRAPH, (graph_entries,))

        with dpg.window(tag=dpg_id, label=f"Graph of {", ".join([i[0] for i in graph_entries])}"):
            x = GraphViewerWindow("", [i[0] for i in graph_entries], "timestamp")
            for i, (_, upd, name) in enumerate(graph_entries):
                allDataUpdaters[upd].add_callback(lambda ts, value, i=i: x.update_graph_data(i, ts, value), dpg_id +
                                                  f"num_{i}", ["timestamp", name])

        self._add_window(dpg_id)
