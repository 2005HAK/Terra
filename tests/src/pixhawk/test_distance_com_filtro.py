import pixhawk as px
import matplotlib.pyplot as plt
import time

sensors = px.Pixhawk()

x = 0
y = 0
z = 0
fx = []
fy = []
fz = []
rx = 0.0006754251140591805
ry = 0.0019773677451885305
rz = 0.0008347233790354948
erm = .1
eix = 0
eiy = 0
eiz = 0
vex = .5
vey = .5
vez = .5
vx = vex * vex + rx
vy = vey * vey + ry
vz = vez * vez + rz
cont = []
i = 0

while i < 1000:
    t_i = time.time()
    sensors.update_data()

    gkx = vx / float(vx + erm)
    gky = vy / float(vy + erm)
    gkz = vz / float(vz + erm)

    estado_atualx = eix + gkx * (sensors.get_vel()[0] - eix)
    estado_atualy = eiy + gky * (sensors.get_vel()[1] - eiy)
    estado_atualz = eiz + gkz * (sensors.get_vel()[2] - eiz)

    variacao_estado_atualx = (1 - gkx) * vx
    variacao_estado_atualy = (1 - gky) * vy
    variacao_estado_atualz = (1 - gkz) * vz

    eix = estado_atualx
    eiy = estado_atualy
    eiz = estado_atualz

    vx = variacao_estado_atualx + rx
    vy = variacao_estado_atualy + ry
    vz = variacao_estado_atualz + rz

    fx.append(estado_atualx)
    fy.append(estado_atualy)
    fz.append(estado_atualz)
    cont.append(i)
    i += 1
    t_f = time.time()

    x += fx[-1] * (t_f - t_i)
    y += fy[-1] * (t_f - t_i)
    z += fz[-1] * (t_f - t_i)

    print(f"Distancia percorrida: x: {x}, y: {y}, z: {z}")

plt.plot(cont, fx, "-b")
plt.plot(cont, fy, "-g")
plt.plot(cont, fz, "-r")
plt.show()