import pixhawk as px
import matplotlib.pyplot as plt
import time

sensors = px.Pixhawk()

vx = 0
vy = 0
vz = 0
dx = 0
dy = 0
dz = 0
cont = []
i = 0
t_i = time.time()
t_f = time.time()
while t_f - t_i < 2:
    sensors.update_data()

    # print(f"Vel: {sensors.get_vel()}")

    vx += sensors.get_vel()[0]
    vy += sensors.get_vel()[1]
    vz += sensors.get_vel()[2]
    cont.append(i)
    i += 1
    t_f = time.time()

dx = vx / i
dy = vy / i
dz = vz / i

print(t_f - t_i)
print(dx, dy, dz)
print("Em Vm 'real': 0.15m/s")