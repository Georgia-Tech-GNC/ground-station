import dearpygui.dearpygui as dpg
from typing import Any, override
import numpy as np
from utils import quaternion_to_euler, multiply_quaternion
from abc import ABC, abstractmethod
from enum import Enum


class Viewport(ABC):

    @abstractmethod
    def set_data(self, data: Any):
        """
        Set the data shown in this viewport

        :param data: The new data to show
        """
        raise NotImplementedError("Viewport set_data not implemented")


class DataTypes(Enum):
    """Represents different types of data that can be shown in a viewport"""

    FLOAT = 1
    QUATERNION = 2
    VECTOR = 3
    STRING = 4

DATATYPE_TO_CONTAINER_CLASS = {}

class DataViewerWindow:
     
    def __init__(
        self, 
        data_labels: list[str | tuple[str, str]],
        types: list[DataTypes]    
    ):
        """
        Creates a new window that displays active data

        :param data_labels: A list of labels
                            Each label should be either a string (representing the name of this item)
                            or a tuple of two strings (the first being the name, and the second being the unit)
        :param types:       A list of DataTypes representing the type of each element in this data viewer window
        """
        self.data_containers: list[Viewport | None] = [None] * len(data_labels)
        self.types = types

        for i, name in enumerate(data_labels):
            self.data_containers[i] = DataViewerWindow._create_container(name, types[i])

    def set_all_values(self, values: list[Any]):
        """
        Sets the value of all items in the data viewer window, in the order that they were initialized

        :param values: a list of all data values to set
        """
        if len(values) != len(self.data_containers):
            print(f"Warning! set_all_values with list of {len(values)} items instead of {len(self.data_containers)}")
            return
    
        for i, value in enumerate(values):
            self.data_containers[i].set_data(value)

    def set_value(self, value_index: int, value: Any):
        """
        Set the value of any item in the data viewer window

        :param value_index:     The index of the item to change
        :param value:           The new value to set
        """
        self.data_containers[value_index].set_data(value)

    @staticmethod
    def _create_container(label: str | tuple, data_type: DataTypes):
        """
        Create a new line in the data viewer

        :param label:       Either a string (representing the name of this item)
                            or a tuple of two strings (the first being the name, and the second being the unit)
        :param data_type:   The type of data that this row should show
        :return:            A reference to the Viewport class responsible for this line
        """
        container_type = DATATYPE_TO_CONTAINER_CLASS[data_type]

        if isinstance(label, tuple):
            return container_type(label[0], units=label[1])
        else:
            return container_type(label)



class FloatViewport(Viewport):

    def __init__(self, name: str, units: str = ""):
        """
        Creates a new viewport that displays float values

        :param name:    the name of this value
        :param units:   the units associated with this value
        """
        self.tag = dpg.generate_uuid()

        with dpg.group(horizontal=True):
            dpg.add_text(name + ":")
            dpg.add_input_double(label="", tag=self.tag, readonly=True, width = 100, step=0)
            if units != "":
                dpg.add_text(units)

    @override
    def set_data(self, data: float):
        dpg.set_value(self.tag, data)


class Vector3Viewer(Viewport):

    def __init__(self, name: str, units: str = ""):
        """
        Creates a new viewport that displays vector3 values

        :param name:    the name of this value
        :param units:   the units associated with this value
        """
        self.tags = [dpg.generate_uuid() for _ in range(3)]

        with dpg.group(horizontal=True):
            if units != "":
                dpg.add_text(f"{name}(X Y Z, {units}):")
            else:
                dpg.add_text(f"{name}(X Y Z):")

            dpg.add_input_double(readonly=True, width=75, step=0, tag=self.tags[0])
            dpg.add_input_double(readonly=True, width=75, step=0, tag=self.tags[1])
            dpg.add_input_double(readonly=True, width=75, step=0, tag=self.tags[2])

    @override
    def set_data(self, data: np.ndarray):
        dpg.set_value(self.tags[0], data[0])
        dpg.set_value(self.tags[1], data[1])
        dpg.set_value(self.tags[2], data[2])


class QuaternionViewport(Viewport):

    def __init__(self, name: str, units: str = ""):
        """
        Creates a new viewport that displays quaternion values (converted to ZYX euler angles)

        :param name:    the name of this value
        :param units:   the units associated with this value
        """
        self.tags = [dpg.generate_uuid() for _ in range(3)]
        self.offset = np.array([1, 0, 0, 0], dtype=np.float32)
        self.most_recent_data = self.offset.copy()

        with dpg.group(horizontal=True):
            if units != "":
                dpg.add_text(f"{name}(Roll Pitch Yaw, {units}):")
            else:
                dpg.add_text(f"{name}(Roll Pitch Yaw):")

            dpg.add_input_double(readonly=True, width=75, step=0, tag=self.tags[0])
            dpg.add_input_double(readonly=True, width=75, step=0, tag=self.tags[1])
            dpg.add_input_double(readonly=True, width=75, step=0, tag=self.tags[2])
            dpg.add_button(label='Zero', callback=self._set_new_offset)

    @override
    def set_data(self, data: np.ndarray):
        self.most_recent_data = data

        rotated_data = multiply_quaternion(self.offset, data)
        a, b, c = quaternion_to_euler(rotated_data)
        dpg.set_value(self.tags[0], a)
        dpg.set_value(self.tags[1], b)
        dpg.set_value(self.tags[2], c)

    def _set_new_offset(self):
        """Resets the current angle to be zero"""
        self.offset = self.most_recent_data * np.array([1, -1, -1, -1])
        self.set_data(self.most_recent_data)

class StringViewport(Viewport):
    
    def __init__(self, name: str, units: str = ""):
        self.tag = dpg.generate_uuid()

        with dpg.group(horizontal=True):
            dpg.add_text(name + ":")
            dpg.add_input_text(label="", tag=self.tag, readonly=True, width = 200)
            if units != "":
                dpg.add_text(units)

    @override
    def set_data(self, data: str):
        dpg.set_value(self.tag, data)


DATATYPE_TO_CONTAINER_CLASS[DataTypes.FLOAT] = FloatViewport
DATATYPE_TO_CONTAINER_CLASS[DataTypes.VECTOR] = Vector3Viewer
DATATYPE_TO_CONTAINER_CLASS[DataTypes.QUATERNION] = QuaternionViewport
DATATYPE_TO_CONTAINER_CLASS[DataTypes.STRING] = StringViewport