from gui_window import GUIWindow


def bar():
    pass

#@GUIWindow.register(name='Foo Window', menu_builder=bar)
class Foo:
    def __init__(self):
        print('Called foo init')

Foo = GUIWindow.register(...)(Foo)




x = Foo()
print(isinstance(x, Foo))

