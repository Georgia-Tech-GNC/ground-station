import dearpygui.dearpygui as dpg
from dearpygui.dearpygui import generate_uuid

MAP_TILES: list[tuple[int | str, tuple[float, float], tuple[float, float]]] = []

class MapViewerWindow:

    def __init__(self):
        if len(MAP_TILES) == 0:
            MapViewerWindow.setup_all_map_tiles()

        with dpg.plot(no_title=True, height=-1, width=-1, anti_aliased=False):
            dpg.add_plot_axis(dpg.mvXAxis)
            with dpg.plot_axis(dpg.mvYAxis):
                for tag, min_bound, max_bound in MAP_TILES:
                    dpg.add_image_series(tag, min_bound, max_bound)

                dpg.add_line_series([-84.398, -84.402, -84.4, -84.398], [33.774, 33.774, 33.776, 33.774])

    @staticmethod
    def setup_all_map_tiles():
        # TODO
        width, height, channels, data = dpg.load_image("map.png")

        with dpg.texture_registry(show=True):
            MAP_TILES.append((
                dpg.add_static_texture(width, height, data),
                (-84.405, 33.771),
                (-84.39, 33.78)
            ))


dpg.create_context()

with dpg.window(label="Outer Map"):
    MapViewerWindow()

for i in range(100):
    print(generate_uuid())

dpg.create_viewport(title='Custom Title', width=1400, height=1400)
dpg.setup_dearpygui()
dpg.show_viewport()
dpg.start_dearpygui()
dpg.destroy_context()
