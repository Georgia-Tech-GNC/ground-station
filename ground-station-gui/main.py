import dearpygui.dearpygui as dpg
import numpy as np
from gui_templates.DataViewerWindow import DataViewerWindow, DataTypes
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


dataNameToIndexAndType = {
    "altitude": (0, float),
    "velocity": (1, np.ndarray),
    "current": (2, float),
    "attitude": (3, np.ndarray),
    "health-status": 4
}

# def set_data(dvw: "DataViewerWindow", dataName: str, )


"""
PLACEHOLDER FUNCTIONS
"""


def idle_to_ground():
    print("Sleep Mode to Ground Mode")

def ground_to_armed():
    print("Ground Mode to Armed Mode")

def ignite_pyro():
    print("IGNITE PYRO!")

def flash_data():
    print("Flash Data from Flight Chip to SD Card")


def expand_health_status():
    print("Expand Health Status")


"""
CREATING THE GUI
"""

dpg.create_context()
dpg.configure_app(docking=True, docking_space=True)

with dpg.window(label="Data Viewer",width=500, height=250, tag="data_viewer", pos=(50, 50)):
    dvw = DataViewerWindow([("Altitude", "meters"), ("Velocity", "m/s"), ("Current", "amps"), "Attitude", "Health Status"], [DataTypes.FLOAT, DataTypes.VECTOR, DataTypes.FLOAT, DataTypes.QUATERNION, DataTypes.STRING])
    
with dpg.window(label="Commands to Rocket",width=300, height=200, tag="rocket_commands", pos=(1000, 50)):
    RocketCommands(["Idle to Ground", "Ground to Armed","IGNITE PYRO", "Flash Data"], [idle_to_ground, ground_to_armed, ignite_pyro, flash_data])

with dpg.window(label="Altitude Graph", width=400, height=400, tag="altitude_graph", pos=(50, 400)):
    a1g = AltitudeGraph("Altitude Graph", "Time (s)", "Altitude (m)")

with dpg.window(label="Velocity Graph", width=400, height=400, tag="velocity_graph", pos=(500, 400)):
    vg = VelocityGraph("Velocity Graph", "Time (s)", "Velocity (m/s)")

with dpg.window(label="Attitude Graph", width=400, height=400, tag="attitude_graph", pos=(950, 400)):   
    a2g = AttitudeGraph("Attitude Graph", "Time (s)", "Attitude")

<<<<<<< HEAD
=======
           

# with dpg.window(label="Debugger Window", width=300, height=300, tag="debugger_window", pos=(600, 50)):
#     pass

# with dpg.window(label="Graph Docking Space", width=700, height=400, tag="graph_docking_window", pos=(50, 400)):
#     pass

>>>>>>> origin/main
a1g.update_graph_data(x_scale, y1)
vg.update_graph_data(x_scale, y2)
a2g.update_graph_data(x_scale, y3)

dvw.set_value(0, y1[-1])
dvw.set_value(1, np.array([0.2345, -.1451, .412]))
dvw.set_value(2, y3[-1])
dvw.set_value(3, np.array([0.2807942, 0.2685945, -0.5123368, 0.7658476]))
dvw.set_value(4, "Armed")


dpg.create_viewport(title='GNC Ground Station', width=1400, height=900)
# dpg.maximize_viewport()

log = mvLogger(dpg.add_window(label='Debug Log', pos=(600, 50), width=350, height=200))
log.log("Initialized Ground Station")
log.log_warning("NO RASPBERRY PI CONNECTED")
log.log_critical("ABORTING!!")

dpg.setup_dearpygui()
dpg.show_viewport()

dpg.start_dearpygui()

dpg.destroy_context()
