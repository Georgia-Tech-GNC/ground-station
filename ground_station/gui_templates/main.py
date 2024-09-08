import dearpygui.dearpygui as dpg
from gui_templates.DataViewerWindow import DataViewerWindow
from gui_templates.GraphViewerWindow import GraphViewerWindow
from gui_templates.ButtonInput import ButtonInput

def say_hi():
    print("Hello World!!!!")

def say_foo():
    print("Foo? Foo?? Foo!!!")

dpg.create_context()

with dpg.window(label="Graph Window",width=800, height=600, tag="graph_window"):
    dvw = DataViewerWindow(["a", "b", "c"])

with dpg.window(label="Graph Window",width=800, height=600, tag="jasdf"):
    gvw = GraphViewerWindow("a", ["a", "b", "c"], "label")

with dpg.window(label="Graph Window",width=800, height=600, tag="jfdas"):
    bi = ButtonInput(["Hello World", "Foo"], [say_hi, say_foo])


# dpg.show_metrics()


dpg.configure_app(docking=True, docking_space=False)

dpg.create_viewport(title='Custom Title', width=850, height=640)

dpg.setup_dearpygui()
dpg.show_viewport()

dpg.start_dearpygui()

dpg.destroy_context()