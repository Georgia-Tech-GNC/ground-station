import dearpygui.dearpygui as dpg
from typing import Any, Callable

class DataViewerWindow:
     
    def __init__(
        self, 
        data_labels: list[str],
        button_callbacks: list[Callable]    
    ):
        self.data_viewer_tags = [dpg.generate_uuid() for _ in range(len(data_labels))]
        self.button_tags = [dpg.generate_uuid() for _ in range(len(button_callbacks))]

        for i, name in enumerate(data_labels):
            with dpg.group(horizontal=True):
                dpg.add_text(name)
                dpg.add_input_double(label="", tag=self.data_viewer_tags[i], readonly=True)
                dpg.add_button(label="Expand", callback=button_callbacks[i], tag=self.button_tags[i])


    def set_value(self, value_index: int, value: Any):
        dpg.set_value(self.data_viewer_tags[value_index], value)

