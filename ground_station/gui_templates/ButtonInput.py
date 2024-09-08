import dearpygui.dearpygui as dpg
from typing import Callable

class ButtonInput:

    def __init__(self, button_names: list[str], button_callbacks: list[Callable]):
        self.button_tags = [dpg.generate_uuid() for _ in range(len(button_names))]

        for i in range(len(button_names)):
            dpg.add_button(label=button_names[i], callback=button_callbacks[i], tag=self.button_tags[i])