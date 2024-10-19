import dearpygui.dearpygui as dpg
from typing import Union, Callable
from gui_templates import GraphViewerWindow

class GraphManager:

    def __init__(self, values: list[str]):
        self.data: dict[str, float] = {value: [] for value in values}
        self.active_windows = []

    def add_new_data(self, data_categery: str, data: float):
        if data_categery not in self.data:
            #TODO: ADD WARNING
            return
        
        self.data[data_categery].append(data)

    def register_new_graph_window(self, w):
        self.active_windows.append(w)
    
    def deregister_graph_window(self, w):
        self.active_windows.remove(w)

    def create_graph_window(self, categories: list[str]):
        if any(i not in self.data.keys() for i in categories): 
            # TODO: log warning
            return
        
        GraphWindow(categories, self)
        
            

class GraphWindow:

    def __init__(self, names: str, parent: GraphManager):
        self.parent = parent
        parent.register_new_graph_window(self)

        self.x_axis_tag = dpg.generate_uuid()
        self.y_axis_tags = [dpg.generate_uuid() for _ in range(len(names))]
        self.line_series_tag = [dpg.generate_uuid() for _ in range(len(names))]

        with dpg.window(label=f"Graph Viewer: {", ".join([i.title() for i in names])}", width=400, height=400, on_close=lambda: parent.deregister_graph_window(self)):
            with dpg.plot(label=", ".join([i.title() for i in names])):
                dpg.add_plot_legend()

                # REQUIRED: create x and y axes, set to auto scale.
                dpg.add_plot_axis(dpg.mvXAxis, label="time", tag=self.x_axis_tag)

                for i, name in enumerate(names):
                    dpg.add_plot_axis(dpg.mvYAxis, label=name, tag=self.y_axis_tags[i])

                    dpg.add_line_series(x=[], y=[], label=name, parent=self.y_axis_tags[i], tag=self.line_series_tag[i])
