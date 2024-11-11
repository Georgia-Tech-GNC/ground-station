import dearpygui.dearpygui as dpg
import requests

WEB_SERVER_URL = "http://0.0.0.0:8080/"

class LoggerWindow:

    def __init__(self, intended_width=500):
        
        self.log_level = 1
        self._auto_scroll = True
        self.filter_id = None

        self.msgs_seen = 0
        self.max_char = round(11.0/75.0 * (intended_width - 500) + 52)

        self.count = 0
        self.flush_count = 1000

        with dpg.group(horizontal=True):
            dpg.add_checkbox(label="Auto-scroll", default_value=True, callback=lambda sender:self.auto_scroll(dpg.get_value(sender)))
            dpg.add_button(label="Clear", callback=lambda: dpg.delete_item(self.filter_id, children_only=True))

        dpg.add_input_text(label="Filter", callback=lambda sender: dpg.set_value(self.filter_id, dpg.get_value(sender)))
        self.child_id = dpg.add_child_window(autosize_x=True, autosize_y=True)
        self.filter_id = dpg.add_filter_set(parent=self.child_id)

        with dpg.theme() as self.info_theme:
            with dpg.theme_component(0):
                dpg.add_theme_color(dpg.mvThemeCol_Text, (255, 255, 255, 255))

        with dpg.theme() as self.debug_theme:
            with dpg.theme_component(0):
                dpg.add_theme_color(dpg.mvThemeCol_Text, (11, 227, 137, 255))


        with dpg.theme() as self.warning_theme:
            with dpg.theme_component(0):
                dpg.add_theme_color(dpg.mvThemeCol_Text, (227, 227, 11, 255))

        with dpg.theme() as self.error_theme:
            with dpg.theme_component(0):
                dpg.add_theme_color(dpg.mvThemeCol_Text, (227, 51, 11, 255))

    def auto_scroll(self, value):
        self._auto_scroll = value

    def _log(self, message, level):

        if level < self.log_level:
            return

        self.count+=1

        if self.count > self.flush_count:
            self.clear_log()

        # what is this line for?
        theme = self.info_theme

        if level in [1, 2]:
            message_type = "[DEBUG]  \t"
            theme = self.debug_theme
        elif level == 3:
            message_type = "[WARNING]\t"
            theme = self.warning_theme
        elif level == 4:
            message_type = "[ERROR]  \t"
            theme = self.error_theme

        message_array = [message[i:(i+self.max_char)] for i in range(0, len(message), self.max_char)]

        for i, text in enumerate(message_array):
            if i == 0:
                log_text = message_type + text
            else:
                log_text = "\t\t\t " + text

            new_log = dpg.add_text(log_text, parent=self.filter_id, filter_key=message)
            dpg.bind_item_theme(new_log, theme)
            if self._auto_scroll:
                scroll_max = dpg.get_y_scroll_max(self.child_id)
                dpg.set_y_scroll(self.child_id, -1.0)


    def clear_log(self):
        dpg.delete_item(self.filter_id, children_only=True)
        self.count = 0

    def log_message(self, msg, priority: int):
        self._log(msg, priority)

    def update_log(self):
        try:
            r = requests.get(WEB_SERVER_URL + "api/logs/get", json={"msg_cnt": self.msgs_seen}, timeout=.1)
        except Exception as e:
            print(f"ERROR: exception [{e}] when calling requests.get()")
            return
    
        if r.status_code == 200:
            try:
                logs = r.json(strict=False)
            except Exception as e:
                print(f"ERROR: exception [{e}] when calling r.json(strict=False)")
                return
            
            for value in logs.values():
                self._log(value["value"], int(value["priority"]))
            
            try:
                self.msgs_seen = max(map(int, logs.keys()))
            except Exception as e:
                print(f"ERROR: exception [{e}] when finding maximum of ids")
