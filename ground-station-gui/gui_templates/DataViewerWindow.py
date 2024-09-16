import dearpygui.dearpygui as dpg
from typing import Any, Callable, override, Type
import numpy as np
from utils import quaternion_to_euler, multiply_quaternion
from abc import ABC, abstractmethod
from enum import Enum


class Viewport(ABC):
    
    @abstractmethod
    def set_data(self, data: Any):
        raise NotImplementedError("Viewport set_data not implemented")


class DataTypes(Enum):
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
        self.data_containers: list[Viewport | None] = [None] * len(data_labels)
        self.types = types

        for i, name in enumerate(data_labels):
            self.data_containers[i] = DataViewerWindow._create_container(name, types[i])

    def set_value(self, value_index: int, value: Any):
        self.data_containers[value_index].set_data(value)

    @staticmethod
    def _create_container(label: str | tuple, dataType: DataTypes):
        ContainerType = DATATYPE_TO_CONTAINER_CLASS[dataType]

        if isinstance(label, tuple):
            return ContainerType(label[0], units=label[1])
        else:
            return ContainerType(label)



class FloatViewport(Viewport):

    def __init__(self, name: str, units: str = ""):
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
        print(rotated_data)
        a, b, c = quaternion_to_euler(rotated_data)
        dpg.set_value(self.tags[0], a)
        dpg.set_value(self.tags[1], b)
        dpg.set_value(self.tags[2], c)

    def _set_new_offset(self):
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
