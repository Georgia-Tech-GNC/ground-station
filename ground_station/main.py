import dearpygui.dearpygui as dpg
from gui_templates.DataViewerWindow import DataViewerWindow

dpg.create_context()

with dpg.window(label="Graph Window",width=800, height=600, tag="graph_window"):
    dvw = DataViewerWindow(["a", "b", "c"])


dpg.show_metrics()


dpg.enable_docking(dock_space=False)

dpg.create_viewport(title='Custom Title', width=850, height=640)

dpg.setup_dearpygui()
dpg.show_viewport()

dpg.start_dearpygui()

dpg.destroy_context()