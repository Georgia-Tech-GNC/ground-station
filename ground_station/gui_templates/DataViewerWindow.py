import dearpygui.dearpygui as dpg
from typing import Any

class DataViewerWindow:
     
    def __init__(
        self, 
        data_labels: list[str],    
    ):
        self.data_viewer_tags = [dpg.generate_uuid() for _ in range(len(data_labels))]

        for i, name in enumerate(data_labels):
            with dpg.group(horizontal=True):
                dpg.add_text(name)
                dpg.add_input_double(label="", tag=self.data_viewer_tags[i], readonly=True)

    def set_value(self, value_index: int, value: Any):
        dpg.set_value(self.data_viewer_tags[value_index], value)

