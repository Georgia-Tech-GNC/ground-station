import requests

r = requests.get('http://0.0.0.0:8080/api/sum', json={"vala":1.32414, "valb": 2.4123})
print(r.status_code)
print(r.json())
