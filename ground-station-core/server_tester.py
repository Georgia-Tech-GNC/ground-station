import requests

r = requests.get('http://0.0.0.0:8080/api/data/MasterElectric')
print(r.status_code)
print(r.json())

