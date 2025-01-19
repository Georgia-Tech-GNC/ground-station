import dearpygui.dearpygui as dpg


class IntegerSlider:

    def __init__(self, max: int, min: int = 1):

        self.id = dpg.generate_uuid()
        dpg.add_slider_int(tag=self.id, min_value=min, max_value=max, clamped=True)


    def get_int(self):
        return dpg.get_value(self.id)
    
    def new_max(self, new_max: int):
        dpg.configure_item(self.id, max=new_max)
    
    def new_min(self, new_min: int):
        if new_min < 1:
            print("ERROR: min must be at least 1")

        dpg.configure_item(self.id, min=new_min)