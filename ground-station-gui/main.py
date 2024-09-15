import dearpygui.dearpygui as dpg
import numpy as np
from gui_templates.DataViewerWindow import DataViewerWindow
from gui_templates.RocketCommandsWindow import RocketCommands
from gui_templates.AltitudeGraphWindow import AltitudeGraph
from gui_templates.VelocityGraphWindow import VelocityGraph
from gui_templates.AttitudeGraphWindow import AttitudeGraph
from dearpygui_ext.logger import mvLogger

x_scale = np.linspace(0, 100)
y1 = np.sin(x_scale)
y2 = np.cos(x_scale)
y3 = np.log(x_scale) + y2

"""
GRAPH FUNCTIONS
"""

# def expand_altitude():
#     with dpg.window(tag="graph_docking_window"):
#         AltitudeGraph("Altitude Graph", "Time (s)", "Altitude (m)")

# def expand_velocity():
#     with dpg.window(label="Velocity", tag="graph_docking_window"):
#         VelocityGraph("Velocity Graph", "Time (s)", "Velocity (m/s)")

# def expand_attitude():
#     with dpg.window(label="Attitude", tag="graph_docking_window"):
#         AttitudeGraph("Attitude Graph", "Time (s)", "Attitude")

"""
PLACEHOLDER FUNCTIONS
"""

def expand_altitude():
    print("a")

def expand_velocity():
    print("a")

def expand_attitude():
    print("a")

def show_health():
    print("Show Health Status")

def sleep_to_ground():
    print("Sleep Mode to Ground Mode")

def ground_to_armed():
    print("Ground Mode to Armed Mode")

def expand_health_status():
    print("Expand Health Status")


"""
CREATING THE GUI
"""

dpg.create_context()
dpg.configure_app(docking=True, docking_space=True)

with dpg.window(label="Data Viewer",width=500, height=250, tag="data_viewer", pos=(50, 50)):
    dvw = DataViewerWindow(["Altitude", "Velocity", "Attitude", "Health Status"], [expand_altitude, expand_velocity, expand_attitude, expand_health_status])
    
with dpg.window(label="Commands to Rocket",width=300, height=200, tag="rocket_commands", pos=(1000, 50)):
    RocketCommands(["Sleep to Ground", "Gound to Armed"], [sleep_to_ground, ground_to_armed])

with dpg.window(label="Altitude Graph", width=400, height=400, tag="altitude_graph", pos=(50, 400)):
    a1g = AltitudeGraph("Altitude Graph", "Time (s)", "Altitude (m)")

with dpg.window(label="Velocity Graph", width=400, height=400, tag="velocity_graph", pos=(500, 400)):
    vg = VelocityGraph("Velocity Graph", "Time (s)", "Velocity (m/s)")

with dpg.window(label="Attitude Graph", width=400, height=400, tag="attitude_graph", pos=(950, 400)):   
    a2g = AttitudeGraph("Attitude Graph", "Time (s)", "Attitude")

# with dpg.window(label="Debugger Window", width=300, height=300, tag="debugger_window", pos=(600, 50)):
#     pass

# with dpg.window(label="Graph Docking Space", width=700, height=400, tag="graph_docking_window", pos=(50, 400)):
#     pass

a1g.update_graph_data(x_scale, y1)
vg.update_graph_data(x_scale, y2)
a2g.update_graph_data(x_scale, y3)

dvw.set_value(0, y1[-1])
dvw.set_value(1, y2[-1])
dvw.set_value(2, y3[-1])


dpg.create_viewport(title='Custom Title', width=1400, height=900)
# dpg.maximize_viewport()

log = mvLogger()
log.log("Initialized Ground Station")
log.log_warning("NO RASBERRY PI CONNECTED")
log.log_critical("ABORTING!!")

dpg.setup_dearpygui()
dpg.show_viewport()

dpg.start_dearpygui()

dpg.destroy_context()