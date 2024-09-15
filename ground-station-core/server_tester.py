import requests
r = requests.get('http://0.0.0.0:8080/api/sum', json={'num1':123, 'num2':124})
print(r.status_code)
print(r.json())
