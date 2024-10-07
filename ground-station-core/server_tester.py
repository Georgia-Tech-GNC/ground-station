import requests

r = requests.get('http://0.0.0.0:8080/api/sum', json={"vala":1.32414, "valb": 2.4123})
print(r.status_code)
print(r.json())




import requests

WEB_SERVER_URL = "http://0.0.0.0:8080"

r = requests.get(WEB_SERVER_URL + "/api/data/RocketKinematics")
if (r.status_code == 200):
    print(r.json())
    d = r.json()
    aw = float(d['attitude_w'])
    ax = float(d['attitude_x'])
    ay = float(d['attitude_y'])
    az = float(d['attitude_z'])
    vel = int(d['velocity'])



def set_attitude_w(data: float):
    pass

class Foo:
    def __init__(self, url, values : dict[str, (type, function)]):
        self.url ...

    def update(self):
        requests.get(WEB_SERVER_URL + self.url)
        if (r, ad... ):
            



allDataUpdaters = [
    rocketDataUpdate = Foo("/api/rocketData/", {
        'attitude_w': [float, set_attitude_w],
    }),
    barDataUpdate = Foo({
        ...
    })
]


for i in allDataUpdaters: i.update()