import requests
import logging

WEB_SERVER_URL = "http://0.0.0.0:8080"
TIMEOUT = 0.1

"""
SETTING UP LOGGER
"""
logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)
formatter = logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
ch.setFormatter(formatter)

logger.addHandler(ch)


def send_cmd_with_id(cmd_id: int):
    try: 
        r = requests.get(f'{WEB_SERVER_URL}/api/command/send', json={"id": cmd_id}, timeout=TIMEOUT)
        if (r.status_code != 200):
            print(f"Command web api call with command id {cmd_id} failed! (bad status code {r})")
    except Exception as e:
        print(f"Command web api call with command id {cmd_id} failed! (exception: {e})")



class DataUpdater:

    def __init__(self, url: str, values: dict[str, type]):

        """
        Creates a new DataUpdater responsible for updating a corresponding DataViewerWindow

        :param url: the specific path being requested on the server
        :type url: str

        :param values: key-value pairs in which the key is the name of a data category
                       in the window and the value is the type of that key
        :type values: dict[str, type]
        """

        self.url = url
        self.values = values

        """
        self.callbacks is to be structured as follows:

        - **self.callbacks**: A dictionary of key-value pairs. The key is
          either an int or a str representing the callback's id. The value is
        – a two-element tuple that consists of:
        - **callable**: The callback function to be executed.
        - **list[str]**: A list of strs that represent the keys required from the
                         return JSON of the API call, in the order expected by
                         the callable.

        The structure can be represented as:
            dict[int | str, tuple[callable, list[str]]]
        """
        self.callbacks: dict[id | str, tuple[callable, list[str]]] = {}

    def is_valid_data_name(self, data_name: str) -> bool:
        return data_name in self.values

    """
    Adds a callback function, along with the id and requisite arguments, to self.callbacks.

    :param func: The callback function to be added.
    :type func: callable
    :param id: The unique id of this callback.
    :type id: int | str
    :param data_names: A list of strings that represent the specific keys needed
                       from the JSON returned by the API call
    :type data_names: list[str]
    """

    def add_callback(self, func: callable, dpg_id: int | str, data_names: list[str]):
        if not callable(func):
            logger.error(f"func [{func}] is not callable.")
            return

        if not isinstance(dpg_id, (int, str)) or isinstance(dpg_id, bool):
            logger.error(f"id [{dpg_id}] is not an int or a str.")
            return

        if dpg_id in self.callbacks:
            logger.error(f"id [{dpg_id}] already exists in (self.callbacks).")
            return

        if not isinstance(data_names, list):
            logger.error(f"data names argument is not a list.")
            return
        
        for data_name in data_names:
            if not isinstance(data_name, str):
                logger.error(f"data name [{data_name}] is not a string.")
                return

            if data_name not in self.values:
                logger.error(f"data name [{data_name}] not in (self.values).")
                return

        # potentially add check regarding the length of data names compared to
        # the number of expected arguments for func? (use inspect module)

        self.callbacks[dpg_id] = (func, data_names)
        logger.info(f"callback with id [{dpg_id}] added succesfully.")

    

    def update(self):
        """
        Updates data by making a GET request to self.url and processes the
        returned JSON data. Calls callback functions from self.callbacks with the 
        relevant data extracted from the JSON response.

        The method performs the following steps:
        1. Makes a GET request to the URL.
        2. Handles errors related to invalid URLs and other exceptions.
        3. Parses the JSON response and extracts required data based on 
        callback functions.
        4. Calls each callback with the corresponding data arguments.
            
        Notes:
            - The method assumes that all data names in the callbacks are
            present in "self.values" based on prior checks in the "add_callback" 
            method.

        """

        try:
            r = requests.get(WEB_SERVER_URL + self.url, timeout=TIMEOUT)
        except requests.exceptions.InvalidURL:
            print("ERROR: invalid URL (bad initialization of object)")
            return
        except requests.exceptions.Timeout:
            print("ERROR: timeout exception")
            return
        except Exception as e:
            print(f"ERROR: other exception [{e}]")
            return

        if r.status_code == 200:
            try:
                data = r.json()
            except Exception as e:
                print(f"ERROR: response is not in JSON format -> [{e}]")
                return

            self.invoke_callbacks(data)

        # error messages for bad status codes should be more specific
        elif r.status_code == 401:
            print("ERROR IN THE API CALL (401 error)")

        else:
            print(f"Other error with status code [{r.status_code}]")

    def invoke_callbacks(self, data: dict):

        if type(data) is not dict:
            logger.error(f"data is not a dictionary: [{type(data).__name__}]")
            return
        
        if data == {}:
            logger.error("inputted data dictionary is empty")
            return

        for dpg_id, (callback, data_names) in self.callbacks.items():
            args = []
            skip_outer = False

            for data_name in data_names:

                # data_name is guaranteed to be in self.values based on error check in
                # add_callback() method
                try:
                    datatype = self.values.get(data_name)
                except Exception as e:
                    print(f"Trying to get {data_name} from returned dict in {self.url} failed with exception {e}")

                datum = data.get(data_name)

                if datum is None:
                    logger.error(f"[{data_name}] not in the provided data")
                    skip_outer = True
                    break

                if datum == "":
                    logger.error(f"[{data_name}] key returned blank value")
                    skip_outer = True
                    break
                
                try:
                    casted_datum = datatype(datum)
                except (TypeError, ValueError) as e:
                    print(f"ERROR: invalid casting of datum [{datum}] to [{datatype}]: {e}")
                    skip_outer = True
                    break

                args.append(casted_datum)

            if skip_outer:
                continue

            try:
                callback(*args)
            except Exception as e:
                print(f"Exception raised during callback [{dpg_id}, {callback}]: [{e}]")
                continue

    def clear_callbacks(self):
        print(f"{len(self.callbacks)} callbacks cleared")
        self.callbacks = {}

    def remove_callback(self, dpg_id: int | str):

        if not isinstance(dpg_id, (int, str)):
            print(f"ERROR: id [{dpg_id}] is not an int or a str.")
            return

        removed_callback = self.callbacks.pop(dpg_id, None)

        if removed_callback is None:
            print(f"WARNING: callback with id [{dpg_id}] is not in (self.callbacks).")
        else:
            print(f"Callback [{removed_callback[0]}] with id [{dpg_id}] successfully removed from (self.callbacks).")

    def remove_callbacks(self, ids: list[int | str]):

        if not isinstance(ids, list):
            print(f"ERROR: ids argument is not a list.")
            return

        if not ids:
            print("WARNING: no callbacks to remove.")
            return

        for i, callback_id in enumerate(ids):
            print(f"[{i + 1}]: ", end="")
            self.remove_callback(callback_id)


updaterRSV = DataUpdater("/api/data/RocketStateVector", {
    "velocity_x": float,
    "velocity_y": float,
    "velocity_z": float,
    "attitude_w": float,
    "attitude_x": float,
    "attitude_y": float,
    "attitude_z": float,
    "position_x": float,
    "position_y": float,
    "position_z": float,
    "world_x": float,
    "world_y": float,
    "world_z": float,
    "timestamp": int
})

updaterRSDef = DataUpdater("/api/data/RocketServoDeflection", {
    "servo_deflection_1": float,
    "servo_deflection_2": float,
    "servo_deflection_3": float,
    "servo_deflection_4": float,
    "timestamp": int
})

updaterRS = DataUpdater("/api/data/RocketState", {
    "rocket_state": int,
    "firing_channel_1": int,
    "firing_channel_2": int,
    "firing_channel_3": int,
    "timestamp": int
})

updaterRGEKF = DataUpdater("/api/data/RocketGroundEKF", {
    "pn_matrix_d1": float,
    "pn_matrix_d2": float,
    "pn_matrix_d3": float,
    "pn_matrix_d4": float,
    "pn_matrix_d5": float,
    "pn_matrix_d6": float,
    "timestamp": int
})

updaterRSData = DataUpdater("/api/data/RocketSensorData", {
    "accelerometer_x": float,
    "accelerometer_y": float,
    "accelerometer_z": float,
    "gyro_x": float,
    "gyro_y": float,
    "gyro_z": float,
    "gps_x": float,
    "gps_y": float,
    "gps_z": float,
    "timestamp": int
})

updaterAFD = DataUpdater("/api/data/RocketAnalogFeedbackData", {
    "current_fb_33": int,
    "pyro_0_cont": int,
    "pyro_1_cont": int,
    "pyro_2_cont": int,
    "pyro_channel_deploy": int
})

allDataUpdaters = {
    'updaterRSV': updaterRSV,
    'updaterRSDef': updaterRSDef,
    'updaterRS': updaterRS,
    'updaterRGEKF': updaterRGEKF,
    'updaterRSData': updaterRSData,
    'updaterAFD': updaterAFD
}
