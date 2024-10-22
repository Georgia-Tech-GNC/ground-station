import dearpygui.dearpygui as dpg
from window_manager import WindowManager, WINDOWS
from query_server.DataUpdater import updaterRSV
import threading
import time

def init_gui():
    dpg.create_context()
    dpg.configure_app(docking=True, docking_space=True)

def start_gui():
    dpg.create_viewport(title='GNC Ground Station', width=1400, height=900)
    dpg.setup_dearpygui()
    dpg.show_viewport()
    dpg.start_dearpygui()

def terminate_gui():
    dpg.destroy_context()

manager = WindowManager()

def update(event):

    st = time.time()
    while True:
        if event.is_set():
            print("exiting thread!")
            break

        t = time.time() - st
        updaterRSV.test({
            "velocity_x": t*5,
            "velocity_y": 5,
            "velocity_z": t,
            "angular_velocity_x": t - 3,
            "angular_velocity_y": t*4,
            "angular_velocity_z": t*t,
            "attitude_w": 1,
            "attitude_x": 0,
            "attitude_y": 0,
            "attitude_z": 0,
            "position_x": t/2,
            "position_y": t*2 - 50,
            "position_z": t,
            "timestamp": 5
        })
        

if __name__ == "__main__":
    init_gui()

    with dpg.viewport_menu_bar():
        with dpg.menu(label="Windows"):
            for name in WINDOWS['Data Windows'].keys():
                dpg.add_menu_item(label=name, callback=lambda _1, _2, n: manager.create_data_viewer_window(n), user_data=name)
        
    
    close_event = threading.Event()
    update_thread = threading.Thread(target=update, args=(close_event,))
    update_thread.start()

    start_gui()

    close_event.set()
    update_thread.join
    terminate_gui()