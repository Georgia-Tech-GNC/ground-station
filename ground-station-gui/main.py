import dearpygui.dearpygui as dpg
from window_manager import WindowManager, DATA_WINDOWS, GRAPHABLE_VALUES
from query_server.data_updater import allDataUpdaters
from gui_templates.RocketCommandsWindow import RocketCommands
import threading
import time
import random
from pathlib import Path
import os

SAVE_FOLDER_PATH = os.path.join(os.path.curdir, 'layout_save_files')
PICKLE_PATH = os.path.join(SAVE_FOLDER_PATH, 'window_manager_save.pickle')
INIT_FILE_PATH = os.path.join(SAVE_FOLDER_PATH, 'gui_layout_save.ini')

INTENDED_WIDTH = 400

def init_gui():
    dpg.create_context()
    dpg.configure_app(docking=True, docking_space=True, init_file=INIT_FILE_PATH)


def start_gui():
    dpg.create_viewport(title='GNC Ground Station', width=1400, height=900)
    dpg.setup_dearpygui()
    dpg.show_viewport()
    dpg.start_dearpygui()


def terminate_gui():
    dpg.destroy_context()


manager: WindowManager = None


def update(event):
    while True:
        time.sleep(0.02)

        for updater in allDataUpdaters.values():
            updater.update()
        for _, logger in manager.all_loggers.items():
            logger.update_log()

        if event.is_set():
            print("exiting thread!")
            break



def save_current_state():
    manager.pickle_state(PICKLE_PATH)
    dpg.save_init_file(INIT_FILE_PATH)
    print("save complete!")


if __name__ == "__main__":
    Path(SAVE_FOLDER_PATH).mkdir(parents=True, exist_ok=True)  # Create the layout save folder if it does not exist

    init_gui()

    with dpg.viewport_menu_bar():
        dpg.add_menu_item(label="Save Layout", callback=save_current_state)

        with dpg.menu(label="Data Windows"):
            for name in DATA_WINDOWS.keys():
                dpg.add_menu_item(label=name, callback=lambda _1, _2, n: manager.create_data_viewer_window(n),
                                  user_data=name)

        with dpg.menu(label="Graphs"):
            graph_menu_checkbox_tags = {}

            def generate_graph():
                global graph_menu_checkbox_tags
                params = []
                for tag, value in graph_menu_checkbox_tags.items():
                    if dpg.get_value(tag):
                        params.append(value)
                        dpg.set_value(tag, False)

                if len(params) == 0:
                    print("Must select at least one item to graph")
                    return
                elif len(params) > 3:
                    print("Cannot graph more then three items at once")
                    return

                manager.create_graph_viewer_window(params)

            for i, (name, (updater, updater_param)) in enumerate(GRAPHABLE_VALUES.items()):
                
                if updater is None:
                    dpg.add_text(name)
                else:   
                    dpg_id = dpg.add_checkbox(label=name)
                    graph_menu_checkbox_tags[dpg_id] = (name, updater, updater_param)

            dpg.add_menu_item(label="Create graph", callback=generate_graph)
        
        with dpg.menu(label="Other Windows"):
            dpg.add_menu_item(label='Rocket Commands', callback=lambda: manager.create_rocket_commands_window("Rocket Commands"))
            dpg.add_menu_item(label='Logger', callback=lambda: manager.create_logger_window("Logger", INTENDED_WIDTH))
            dpg.add_menu_item(label='Map', callback=lambda: manager.create_map_viewer_window("Rocket Position Tracker"))


    manager = WindowManager(PICKLE_PATH)

    close_event = threading.Event()
    update_thread = threading.Thread(target=update, args=(close_event,))
    update_thread.start()

    start_gui()

    close_event.set()
    update_thread.join()
    terminate_gui()
