import requests

r = requests.get('http://0.0.0.0:8080/api/data/MasterElectric', timeout=.1)
print(r.status_code)
print(r.json())

r = requests.get('http://0.0.0.0:8080/api/data/MasterElectrc', timeout=.1)
print(r.status_code)
print(r.json())

