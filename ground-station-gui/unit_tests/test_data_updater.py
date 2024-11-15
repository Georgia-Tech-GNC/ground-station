import unittest
import logging
import requests
import requests_mock

# imports the data_updater file for testing
import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "query_server")))
import data_updater

WEB_SERVER_URL = data_updater.WEB_SERVER_URL



class TestDataUpdater(unittest.TestCase):

    # @classmethod
    # def setUpClass(cls):
    #     cls.updater = data_updater.DataUpdater("test/url", {"x": 2, "y": 5})
    
    # @requests_mock.Mocker()
    # def test_send_cmd_with_id_1(self, m):
    #     m.get(f'{WEB_SERVER_URL}api/command/send?id=1', json={})

    """
    is_valid_data_name tests
    """
    def test_is_valid_data_name(self):
        updater = data_updater.DataUpdater("test/url", {"x": str, "y": str, "2": int})

        self.assertTrue(updater.is_valid_data_name("x"))
        self.assertTrue(updater.is_valid_data_name("y"))
        self.assertTrue(updater.is_valid_data_name("2"))
    
    def test_is_invalid_data_name_strings(self):
        updater = data_updater.DataUpdater("test/url", {"x": str, "y": str, "2": int})

        self.assertFalse(updater.is_valid_data_name("a"))
        self.assertFalse(updater.is_valid_data_name("b"))
        self.assertFalse(updater.is_valid_data_name("X"))
        self.assertFalse(updater.is_valid_data_name("Y"))

    def test_is_invalid_data_name_nonstrings(self):
        updater = data_updater.DataUpdater("test/url", {"x": str, "y": str, "2": int})

        self.assertFalse(updater.is_valid_data_name(2))
        self.assertFalse(updater.is_valid_data_name(18.9))
        self.assertFalse(updater.is_valid_data_name(None))
        self.assertFalse(updater.is_valid_data_name(str))
        self.assertFalse(updater.is_valid_data_name(int))

    """
    add_callback tests
    """
    def test_add_callback_noncallable(self):
        updater = data_updater.DataUpdater("test/url", {"x": str, "y": str, "2": int})

        with self.assertLogs("data_updater", level="DEBUG") as log:
            updater.add_callback("func1", 2, ["a", "b", "c"])
            self.assertIn("ERROR:data_updater:func [func1] is not callable.", log.output)

            updater.add_callback(17, 2, ["a", "b", "c"])
            self.assertIn("ERROR:data_updater:func [17] is not callable.", log.output)

            updater.add_callback(None, 2, ["a", "b", "c"]) 
            self.assertIn("ERROR:data_updater:func [None] is not callable.", log.output)

    
    def test_add_callback_bad_id_type(self):
        updater = data_updater.DataUpdater("test/url", {"x": str, "y": str, "2": int})

        bad_dpg_ids = [6.2, 4.5, {"a": "b"}, [2, 3, 4], False, True]

        with self.assertLogs("data_updater", level="DEBUG") as log:

            for bad_dpg_id in bad_dpg_ids:
                updater.add_callback(lambda: None, bad_dpg_id, ["a", "b", "c"])
                self.assertIn(f"ERROR:data_updater:id [{bad_dpg_id}] is not an int or a str.", log.output)

    def test_add_callback_id_repeat(self):
        updater = data_updater.DataUpdater("test/url", {"a": str, "b": str, "c": int})

        dpg_ids = [5, 9, 0, -4, "hello", "peter", 2, 7, 3]

        for dpg_id in dpg_ids:
            updater.add_callback(lambda: None, dpg_id, ["a", "b", "c"])


        with self.assertLogs("data_updater", level="DEBUG") as log:

            for dpg_id in dpg_ids:
                updater.add_callback(lambda: None, dpg_id, ["a", "b", "c"])
                self.assertIn(f"ERROR:data_updater:id [{dpg_id}] already exists in (self.callbacks).", log.output)

    def test_add_callback_data_names_not_list(self):
        updater = data_updater.DataUpdater("test/url", {"x": str, "y": str, "2": int})

        bad_data_namess = ["test", 3, (2, 3), 7, 19.2, {"a": "b"}, None]

        with self.assertLogs("data_updater", level="DEBUG") as log:

            for i in range(len(bad_data_namess)):
                updater.add_callback(lambda: None, i, bad_data_namess[i])
                self.assertIn(f"ERROR:data_updater:data names argument is not a list.", log.output)

    def test_add_callback_data_names_bad_string(self):
        updater = data_updater.DataUpdater("test/url", 
                                           {"t": str, "a": str, "c": str, "187": int, "begl": float, "b": str})

        bad_data_namess = [
            ([1, 2, "t", 3, "a", "c", "187", "begl"], 1), 
            ([1, "a", "t", 189, "a", "c", 187, "begl"], 1), 
            ([(2, 3), 1, "t", 3, "a", "c", "187", "begl"], (2, 3)), 
            (["a", ("t", "a"), "t", 3, "c", "187", "begl"], ("t", "a")), 
            (["b", {"b": 2}, "t", 3, "a", "c", {"a": "b"}, "begl"], {"b": 2}),
        ]

        with self.assertLogs("data_updater", level="DEBUG") as log:

            for i in range(len(bad_data_namess)):
                updater.add_callback(lambda: None, i, bad_data_namess[i][0])
                self.assertIn(f"ERROR:data_updater:data name [{bad_data_namess[i][1]}] is not a string.", log.output)
    

    def test_add_callback_data_names_not_in_values(self):
        updater = data_updater.DataUpdater("test/url", {"a": str, "b": int, "c": str, "d": int, "e": float})

        bad_data_namess = [
            (["a", "b", "c", "d", "e", "f"], "f"),
            (["a", "b", "c", "d", "e", "g"], "g"),
            (["a", "b", "h", "d", "e", "f"], "h"),
            (["a", "i", "c", "d", "e", "f"], "i"),
            (["j", "b", "c", "d", "e", "f"], "j")
        ]

        with self.assertLogs("data_updater", level="DEBUG") as log:

            for i in range(len(bad_data_namess)):
                updater.add_callback(lambda: None, i, bad_data_namess[i][0])
                self.assertIn(f"ERROR:data_updater:data name [{bad_data_namess[i][1]}] not in (self.values).", log.output)
    
    def test_add_callback_success(self):
        updater = data_updater.DataUpdater("test/url", {"a": str, "b": int, "c": str, "d": int, "e": float})

        data_namess = [
            ["a", "b", "c"],
            ["a", "b", "d"],
            ["a", "b", "e"],
            ["a", "c", "b"],
            ["a", "c", "d"],
            ["a", "c", "e"],
            ["a", "d", "b"],
            ["a", "d", "c"],
            ["a", "d", "e"],
            ["a", "e", "b"],
            ["a", "e", "c"],
            ["a", "e", "d"],
            ["b", "a", "c"],
            ["b", "a", "d"],
            ["b", "a", "e"],
            ["b", "c", "a"],
            ["b", "c", "d"],
            ["b", "c", "e"],
            ["b", "d", "a"],
            ["b", "d", "c"],
            ["b", "d", "e"],
            ["b", "e", "a"],
            ["b", "e", "c"],
            ["b", "e", "d"],
            ["c", "a", "b"],
            ["c", "a", "d"],
            ["c", "a", "e"],
            ["c", "b", "a"],
            ["c", "b", "d"],
            ["c", "b", "e"],
            ["c", "d", "a"],
            ["c", "d", "b"],
            ["c", "d", "e"],
            ["c", "e", "a"],
            ["c", "e", "b"],
            ["c", "e", "d"],
            ["d", "a", "b"],
            ["d", "a", "c"],
            ["d", "a", "e"],
            ["d", "b", "a"],
            ["d", "b", "c"],
            ["d", "b", "e"],
            ["d", "c", "a"],
            ["d", "c", "b"],
            ["d", "c", "e"],
            ["d", "e", "a"],
            ["d", "e", "b"],
            ["d", "e", "c"],
            ["e", "a", "b"],
            ["e", "a", "c"],
            ["e", "a", "d"],
            ["e", "b", "a"],
            ["e", "b", "c"],
            ["e", "b", "d"],
            ["e", "c", "a"],
            ["e", "c", "b"],
            ["e", "c", "d"],
            ["e", "d", "a"],
            ["e", "d", "b"],
            ["e", "d", "c"]
        ]

        with self.assertLogs("data_updater", level="DEBUG") as log:

            for i in range(len(data_namess)):
                updater.add_callback(lambda: None, i, data_namess[i])
                self.assertIn(f"INFO:data_updater:callback with id [{i}] added succesfully.", log.output)

    """
    update tests
    """


    """
    invoke_callbacks tests
    """


    """
    clear_callback tests
    """


    """
    remove_callback tests
    """


    """
    remove_callbacks tests
    """


    

    

    
    

    



if __name__ == "__main__":
    unittest.main()
