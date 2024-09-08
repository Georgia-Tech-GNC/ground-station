import dearpygui.dearpygui as dpg

class GraphViewerWindow:

    def __init__(
        self, 
        graph_name: str, 
        plot_names: list[str],
        x_axis_label: str,
        _height: int = -1, 
        _width: int = -1, 
        add_legend: bool = True
    ):
        self.plot_tag = dpg.generate_uuid()
        self.x_axis_tag = dpg.generate_uuid()
        self.y_axis_tags = [dpg.generate_uuid() for _ in range(len(plot_names))]
        self.line_series_tag = [dpg.generate_uuid() for _ in range(len(plot_names))]

        with dpg.plot(label=graph_name, height=_height, width=_width, tag=self.plot_tag):
            # optionally create legend
            if add_legend: dpg.add_plot_legend()

            # REQUIRED: create x and y axes, set to auto scale.
            dpg.add_plot_axis(dpg.mvXAxis, label=x_axis_label, tag=self.x_axis_tag)

            for i, name in enumerate(plot_names):
                dpg.add_plot_axis(dpg.mvYAxis, label=name, tag=self.y_axis_tags[i])


                # series belong to a y axis. Note the tag name is used in the update
                # function update_data
                dpg.add_line_series(x=[], y=[], label=name, parent=self.y_axis_tags[i], tag=self.line_series_tag[i])

    def update_graph_data(self, graph_index: int, x_data, y_data):
        dpg.set_value(self.line_series_tag[graph_index], [list(x_data), list(y_data)])          
        dpg.fit_axis_data(self.x_axis_tag)
        dpg.fit_axis_data(self.y_axis_tags[graph_index])