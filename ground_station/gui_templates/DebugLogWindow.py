import dearpygui.dearpygui as dpg
from dearpygui_ext.logger import mvLogger

def DebugLog(self):
    """
    Creating our logger window
    """
    window_name = (
        self.logger.replace("_", " ").title()
        + "##window"
    )
    with s.window(
        name=window_name,
        width=500,
        height=500,
        x_pos=500,
        y_pos=350,
        no_scrollbar=True,
    ):
        c.add_logger(
            name=self.logger,
            autosize_x=True,
            autosize_y=True,
        )

def init_windows(self):
    """
    Creates and arranges our windows
    """
    self._logger_window()
    c.set_main_window_size(1000, 850)
    c.set_main_window_pos(x=280, y=0)