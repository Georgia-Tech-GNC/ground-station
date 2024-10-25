import dearpygui.dearpygui as dpg
from typing import Callable


def do_both(a: Callable, b: Callable):
    a()
    b()


class RocketCommands:

    def __init__(self, button_names: list[str | tuple[str, bool]], button_callbacks: list[Callable]):
        self.button_tags = [dpg.generate_uuid() for _ in range(len(button_names))]

        self.popup = None

        for i in range(len(button_names)):
            if isinstance(button_names[i], str):
                dpg.add_button(label=button_names[i], callback=button_callbacks[i], tag=self.button_tags[i])
            elif not button_names[i][1]:
                dpg.add_button(label=button_names[i][0], callback=button_callbacks[i], tag=self.button_tags[i])
            else:
                dpg.add_button(label=button_names[i][0], callback=lambda *args, name=button_names[i][0], callback=button_callbacks[i]: self.open_confirmation_window(name, callback), tag=self.button_tags[i])

        with dpg.popup(parent=dpg.last_item(), modal=True) as popup:
            self.popup_text_id = dpg.add_text("Default popup text")
            with dpg.group(horizontal=True, parent=popup) as group:
                self.popup_confirm_id = dpg.add_button(label="CONFIRM", parent=group)
                dpg.add_button(label="DENY", parent=group, callback=lambda: dpg.hide_item(self.popup))

        self.popup = popup

        
    def open_confirmation_window(self, command: str, callback_on_confirm: Callable): 
        dpg.configure_item(self.popup_confirm_id, callback=
                           lambda: do_both(callback_on_confirm, lambda: dpg.hide_item(self.popup)))
        dpg.configure_item(self.popup_text_id, default_value=f"Comfirm: {command}")
        dpg.show_item(self.popup)
                