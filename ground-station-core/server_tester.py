import requests
import time
import sys


server = sys.argv[1] if len(sys.argv) > 1 else '100.70.37.160'

for i in range(3):
    r = requests.get(f'http://{server}:8080/api/data/RocketSensorData', timeout=.1)
    print(r.status_code)
    print(r.json())
    time.sleep(0.01)


r = requests.get(f'http://{server}:8080/api/data/RocketGroundEKF', timeout=.1)
print(r.status_code)
print(r.json())

s = requests.get(f'http://{server}:8080/api/data/MasterElectrc', timeout=.1)
print(s.status_code)
print(r.json())

r = requests.get(f'http://{server}:8080/api/command/send', json={"id": 1}, timeout=.1)
print(r.status_code)
print(r.json())

r = requests.get(f'http://{server}:8080/api/logs/get', json={"msg_cnt": 3}, timeout=.1)
print(r.status_code)
logs = r.json(strict=False)
for id, value in logs.items():
    print(f"MSG {id} P {value['priority']}: {value['value'].rstrip()}")
print(f"Last msg recv: {max(map(int, logs.keys()))}")


