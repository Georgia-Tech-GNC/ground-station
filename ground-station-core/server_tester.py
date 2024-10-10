import requests
import time

for i in range(3):
    r = requests.get('http://0.0.0.0:8080/api/data/RocketKinematics', timeout=.1)
    print(r.status_code)
    print(r.json())
    time.sleep(0.01)


r = requests.get('http://0.0.0.0:8080/api/data/MasterElectric', timeout=.1)
print(r.status_code)
print(r.json())

s = requests.get('http://0.0.0.0:8080/api/data/MasterElectrc', timeout=.1)
print(s.status_code)
print(r.json())




