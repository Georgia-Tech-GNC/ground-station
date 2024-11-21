import dearpygui.dearpygui as dpg
from dearpygui.dearpygui import generate_uuid
from query_server.data_updater import allDataUpdaters
from math import sqrt

MAP_TILES: list[tuple[int | str, tuple[float, float], tuple[float, float]]] = []

R3O6 = sqrt(3)/6

class MapViewerWindow:

    def __init__(self):
        if len(MAP_TILES) == 0:
            MapViewerWindow.setup_all_map_tiles()

        self.rocket_pos_theme = generate_uuid()
        self.self_id = generate_uuid()

        with dpg.theme(tag=self.rocket_pos_theme):
            with dpg.theme_component(dpg.mvScatterSeries):
                dpg.add_theme_style(dpg.mvPlotStyleVar_Marker, dpg.mvPlotMarker_Circle, category=dpg.mvThemeCat_Plots)
                dpg.add_theme_style(dpg.mvPlotStyleVar_MarkerSize, 8, category=dpg.mvThemeCat_Plots)
                dpg.add_theme_style(dpg.mvPlotCol_MarkerFill, 0, category=dpg.mvThemeCat_Plots)

        with dpg.plot(no_title=True, height=-1, width=-1, anti_aliased=False):
            dpg.add_plot_legend()

            dpg.add_plot_axis(dpg.mvXAxis)
            with dpg.plot_axis(dpg.mvYAxis):

                for tag, min_bound, max_bound in MAP_TILES:
                    dpg.add_image_series(tag, min_bound, max_bound)

                self.rocket_series_tag = dpg.add_scatter_series([-84.4], [33.776], label="Rocket")
                dpg.bind_item_theme(self.rocket_series_tag, self.rocket_pos_theme)

        allDataUpdaters['updaterRSData'].add_callback(lambda a, b: self.update(a, b), self.self_id, ['gps_y', 'gps_x'])

    def update(self, gps_x: float, gps_y: float):
        dpg.set_value(self.rocket_series_tag, [[gps_x], [gps_y]])


    @staticmethod
    def setup_all_map_tiles():
        # TODO
        width, height, channels, data = dpg.load_image("map.png")

        with dpg.texture_registry(show=False):
            MAP_TILES.append((
                dpg.add_static_texture(width, height, data),
                (-84.405, 33.771),
                (-84.39, 33.78)
            ))
