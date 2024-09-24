import pixhawk as px
import matplotlib.pyplot as plt
import time

sensors = px.Pixhawk()

vx = []
vy = []
vz = []
dx = 0
dy = 0
dz = 0
cont = []
i = 0

while i < 1000:
    t_i = time.time()
    sensors.update_data()

    # print(f"Vel: {sensors.get_vel()}")

    vx.append(sensors.get_vel()[0])
    vy.append(sensors.get_vel()[1])
    vz.append(sensors.get_vel()[2])
    cont.append(i)
    i += 1
    t_f = time.time()

    dx += vx[-1] * (t_f - t_i)
    dy += vy[-1] * (t_f - t_i)
    dz += vz[-1] * (t_f - t_i)

    print(f"Distancia percorrida: x: {dx}, y: {dy}, z: {dz}")

plt.plot(cont, vx, "-b")
plt.plot(cont, vy, "-g")
plt.plot(cont, vz, "-r")
plt.show()