import dearpygui.dearpygui as dpg

class AltitudeGraph:

    def __init__(
        self,
        graph_name: str,
        x_axis_label: str,
        y_axis_label: str,
        _height: int = -1,
        _width: int = -1,
        add_legend: bool = True
    ):
        self.plot_tag = dpg.generate_uuid()
        self.x_axis_tag = dpg.generate_uuid()
        self.y_axis_tag = dpg.generate_uuid()
        self.line_series_tag = dpg.generate_uuid()

        with dpg.plot(label=graph_name, height=_height, width=_width, tag=self.plot_tag):
            if add_legend:
                dpg.add_plot_legend()
            
            dpg.add_plot_axis(dpg.mvXAxis, label=x_axis_label, tag=self.x_axis_tag)
            dpg.add_plot_axis(dpg.mvYAxis, label=y_axis_label, tag=self.y_axis_tag)

            # design question: should y_axis_label be the label for the series as well?
            dpg.add_line_series(x=[], y=[], label=y_axis_label, parent=self.y_axis_tag, tag=self.line_series_tag)

    def update_graph_data(self, x_data, y_data):
        dpg.set_value(self.line_series_tag, [list(x_data), list(y_data)])          
        dpg.fit_axis_data(self.x_axis_tag)
        dpg.fit_axis_data(self.y_axis_tag)