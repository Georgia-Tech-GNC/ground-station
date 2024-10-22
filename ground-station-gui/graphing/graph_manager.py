import dearpygui.dearpygui as dpg
from typing import Union, Callable
from gui_templates import GraphViewerWindow

class GraphManager:

    def __init__(self, values: dict[str, Callable[..., tuple[list[float], list[float]]]]):
        self.data: dict[str, Callable[..., tuple[list[float], list[float]]]] = values
        self.active_windows: dict[GraphWindow, list[str]] = {}

    def get_names(self):
        return self.data.keys()

    def register_new_graph_window(self, w, categories: list[str]):
        self.active_windows[w] = categories
    
    def deregister_graph_window(self, w):
        if w in self.active_windows:
            del self.active_windows[w]

    def create_graph_window(self, categories: list[str]):
        if any(i not in self.data.keys() for i in categories): 
            # TODO: log warning
            return
        
        GraphWindow(categories, self)

    def update(self):
        new_data = {k: v() for k, v in self.data.items()}

        for gw, c in self.active_windows.items():
            for i, cat in enumerate(c):
                gw.update_graph_data(i, new_data[cat][0], new_data[cat][1])
        
            

class GraphWindow:

    def __init__(self, names: str, parent: GraphManager):
        self.parent = parent
        parent.register_new_graph_window(self, names)

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

    def update_graph_data(self, graph_index: int, x_data, y_data):
        dpg.set_value(self.line_series_tag[graph_index], [list(x_data), list(y_data)])          
        dpg.fit_axis_data(self.x_axis_tag)
        dpg.fit_axis_data(self.y_axis_tags[graph_index])
