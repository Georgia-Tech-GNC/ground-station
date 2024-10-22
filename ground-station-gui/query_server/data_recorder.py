from gui_templates.DataViewerWindow import DataViewerWindow, DataTypes
from typing import Any

class DataRecorder:

    def __init__(self, data_labels: list[str | tuple[str, str]], types: list[DataTypes]):
        self.data: list[Any] = [[]]*len(data_labels)

        self._build_data_viewer_window_func = lambda: DataViewerWindow(data_labels, types)
        self.active_data_viewer_window: DataViewerWindow = None

    def add_data(self, timestamp: int, new_data: list[Any]):
        if len(new_data) != len(self.data):
            print(f"Passed {len(new_data)} entries into DataRecorder.add_data, but only expected {len(self.data)}")

        for i, current in enumerate(self.data):
            if new_data[i] is not None:
                current.append((timestamp, new_data[i]))
                if self.active_data_viewer_window is not None:
                    self.active_data_viewer_window.set_value(i, new_data[i])

    def get_data_by_index(self, index: int):
        print(type(self.data[index][0][1]))
        return self.data[index]

    def create_window_if_not_active(self):
        if self.active_data_viewer_window is not None: return

        self.active_data_viewer_window = self._build_data_viewer_window_func()

    def close_window_callback(self):
        self.active_data_viewer_window = None

    def is_window_active(self) -> bool:
        return self.active_data_viewer_window is not None

       

        
