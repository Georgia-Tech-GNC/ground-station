from typing import Optional, Callable
import dearpygui.dearpygui as dpg
import functools

class GUIWindow:

    WINDOWS = {}
    MENU_ROOT_ITEMS = {}
    MENU_CUSTOM_BUILDERS = {}

    @staticmethod
    def build_root_menu():
        for name, listing in GUIWindow.MENU_ROOT_ITEMS.items():
            with dpg.menu(label=name):
                for item in listing:
                    dpg.add_menu_item(label=item[0], callback=lambda: GUIWindow.create_window(item[1]))
        for custom_name, impl in GUIWindow.MENU_CUSTOM_BUILDERS.items():
            with dpg.menu(label=custom_name):
                impl()

    def create_window(name):
        with dpg.window(label=name):
            GUIWindow.WINDOWS[name]()
            ## Todo: add the subscriber stuff???

    @staticmethod
    def register(
        *, # The asterick forces all arguments to be used via keywords

        name: Optional[str] = None, is_persistent: bool = True,

        menu_category: Optional[str] = None, menu_name: Optional[str] = None,
        menu_builder: Optional[Callable[[], None]] = None,
    ):
        def register_decorator(cls: type, **kwargs):
            nonlocal name, is_persistent, menu_category, menu_name, menu_builder

            if cls is None or not isinstance(cls, type):
                raise TypeError(f"Registered GUI window must be a class (recieved {cls})")
            
            assert name is None or isinstance(name, str)
            assert isinstance(is_persistent, bool)
            assert menu_category is None or isinstance(menu_category, str)
            assert menu_name is None or isinstance(menu_name, str)
            assert menu_builder is None or isinstance(menu_builder, Callable)
            
            if name is None:
                name = cls.__name__
            if menu_name is None:
                menu_name = name

            if is_persistent and ((menu_name is not None) or (menu_builder is not None)):
                raise TypeError("If is_persistent is true, a menu category or menu builder cannot be specified")
            elif is_persistent != (menu_category is None):
                raise TypeError("Exactly one of is_persistent or menu_category should be specified")
            elif (menu_name is None) != (menu_builder is None):
                raise TypeError("Exactly one of menu_category and menu_name must be specified")
            
            
            print(f"Registered {name}")

            GUIWindow.WINDOWS[cls.__name__.upper()] = cls
            if menu_name is not None:
                if menu_category in GUIWindow.MENU_ROOT_ITEMS:
                    GUIWindow.MENU_ROOT_ITEMS[menu_category].append((menu_name, cls.__name__.upper()))
                else:
                    GUIWindow.MENU_ROOT_ITEMS[menu_category] = [menu_name]
            else:
                GUIWindow.MENU_CUSTOM_BUILDERS[menu_category] = menu_builder
        
            return cls
        
        return register_decorator
    



