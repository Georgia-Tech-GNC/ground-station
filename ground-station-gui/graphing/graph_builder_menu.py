from .graph_manager import GraphManager
import dearpygui.dearpygui as dpg

class GraphBuilderMenu:
    def __init__(self, graph_names: list[str], manager: GraphManager):
        self.names = graph_names
        self.checkbox_ids = [dpg.generate_uuid() for _ in self.names]
        self.manager = manager

        dpg.add_text("Select values to graph:")

        for i, name in enumerate(self.names):
            dpg.add_checkbox(label=name, tag=self.checkbox_ids[i])

        dpg.add_button(label="Create Graph Window", callback=self._create_graph)
        
    def _create_graph(self):
        list_axes: list[str] = []
        for i, name in enumerate(self.names):
            if dpg.get_value(self.checkbox_ids[i]): list_axes.append(name)

        if len(list_axes) == 0: 
            # TODO: log error
            return
        
        for i in self.checkbox_ids: dpg.set_value(i, False)

        self.manager.create_graph_window(list_axes)