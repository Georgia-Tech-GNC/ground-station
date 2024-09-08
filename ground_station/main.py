import dearpygui.dearpygui as dpg
from gui_templates.DataViewerWindow import DataViewerWindow
from gui_templates.GraphViewerWindow import GraphViewerWindow
from gui_templates.ButtonInput import ButtonInput

# placeholder -- to be updated
def show_telemetry():
    print("Show Telemetry Data")

# placeholder -- to be updated
def show_health():
    print("Show Health Status")

# placeholder -- to be updated
def sleep_to_ground():
    print("Sleep Mode to Ground Mode")

# placeholder -- to be updated
def ground_to_armed():
    print("Ground Mode to Armed Mode")



dpg.create_context()

with dpg.window(label="Graph Window",width=800, height=600, tag="graph_window"):
    dvw = DataViewerWindow(["a", "b", "c"])

with dpg.window(label="Graph Window",width=800, height=600, tag="jasdf"):
    gvw = GraphViewerWindow("a", ["a", "b", "c"], "label")

with dpg.window(label="Graph Window",width=800, height=600, tag="jfdas"):
    bi = ButtonInput(["Show Telemetry", "Show Health Status", "Sleep to Ground", "Gound to Armed"], [show_telemetry, show_health, sleep_to_ground, ground_to_armed])


# dpg.show_metrics()


dpg.configure_app(docking=True, docking_space=False)

dpg.create_viewport(title='Custom Title', width=850, height=640)

dpg.setup_dearpygui()
dpg.show_viewport()

dpg.start_dearpygui()

dpg.destroy_context()