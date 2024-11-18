import requests
import time
import dearpygui.dearpygui as dpg

class RocketHealthTracker:

    def __init__(self):
        self.num_successful = 0
        self.num_failed = 0
        self.last_msg_time = 0
        self.connected_to_ground = False

        self.dpg_id_success = dpg.generate_uuid()
        self.dpg_id_fail = dpg.generate_uuid()
        self.dpg_id_connected = dpg.generate_uuid()
        self.dpg_id_drop_rate = dpg.generate_uuid()

        dpg.add_text("CONNECTED TO GROUND STATION", tag=self.dpg_id_connected)

        with dpg.group(horizontal=True):
            dpg.add_text("Packets Recieved Successfully:")
            dpg.add_input_double(label="", tag=self.dpg_id_success, readonly=True, width=100, step=0)

        with dpg.group(horizontal=True):
            dpg.add_text("Corrupted Packets Recieved:")
            dpg.add_input_double(label="", tag=self.dpg_id_fail, readonly=True, width=100, step=0)

        with dpg.group(horizontal=True):
            dpg.add_text("Packet Loss Rate:")
            dpg.add_input_double(label="", tag=self.dpg_id_drop_rate, readonly=True, width=100, step=0)

    def update(self):
        try:
            r = requests.get('http://0.0.0.0:8080/api/packets/info', timeout=0.1)
            if (r.status_code != 200):
                self.connected_to_ground = False
                return
            
            data = r.json()

            if ('succeeded' not in data) or ('failed' not in data):
                self.connected_to_ground = False
                return

            self.num_successful = data['succeeded']
            self.num_failed = data['failed']
            self.last_msg_time = time.time()
            
            self.connected_to_ground = True

            dpg.set_value(self.dpg_id_success, self.num_successful)
            dpg.set_value(self.dpg_id_fail, self.num_failed)
            if self.num_failed + self.num_successful != 0:
                drop_rate = self.num_failed / (self.num_successful + self.num_failed)
            else:
                drop_rate = 0
            dpg.set_value(self.dpg_id_drop_rate, 100*drop_rate)

        except Exception as e:
            self.connected_to_ground = False
            print(e)

        if not self.connected_to_ground:
            dpg.set_value(self.dpg_id_connected, 'DISCONNECTED FROM GROUND STATION')
        else:
            dpg.set_value(self.dpg_id_connected, 'CONNECTED TO GROUND STATION')