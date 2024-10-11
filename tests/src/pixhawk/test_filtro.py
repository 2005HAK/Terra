import pixhawk as px
import matplotlib.pyplot as plt
import time
import numpy as np

sensors = px.Pixhawk()

fx = []
fy = []
fz = []
rx = 0.0006754251140591805
ry = 0.0019773677451885305
rz = 0.0008347233790354948
eix, eiy, eiz = sensors.get_vel()
vex = .5
vey = .5
vez = .5
vx = vex * vex + rx
vy = vey * vey + ry
vz = vez * vez + rz
cont = []

buffer_size = 50
buffer_x = []
buffer_y = []
buffer_z = []

def calculate_dynamic_error(buffer):
    if len(buffer) > 1:
        return np.std(buffer)  # Calcula o desvio padrão das leituras
    else:
        return 0.1  # Valor padrão se não houver dados suficientes

ti = time.time()
td = ti

while td - ti < 10:
    sensors.update_data()
    td = time.time()

    vel_raw = sensors.get_vel()

    # Atualiza buffers com as novas leituras
    if len(buffer_x) >= buffer_size:
        buffer_x.pop(0)  # Remove o mais antigo
    if len(buffer_y) >= buffer_size:
        buffer_y.pop(0)
    if len(buffer_z) >= buffer_size:
        buffer_z.pop(0)

    buffer_x.append(vel_raw[0])
    buffer_y.append(vel_raw[1])
    buffer_z.append(vel_raw[2])

    # Calcula os erros de medição dinamicamente
    ermx = calculate_dynamic_error(buffer_x)
    ermy = calculate_dynamic_error(buffer_y)
    ermz = calculate_dynamic_error(buffer_z)

    gkx = vx / float(vx + ermx)
    gky = vy / float(vy + ermy)
    gkz = vz / float(vz + ermz)

    print(f"Erm: {ermx}, {ermy}, {ermz}")
    print(f"Velocidade Bruta:  {vel_raw[0]}, {vel_raw[1]}, {vel_raw[2]}")

    estado_atualx = eix + gkx * (vel_raw[0] - eix)
    estado_atualy = eiy + gky * (vel_raw[1] - eiy)
    estado_atualz = eiz + gkz * (vel_raw[2] - eiz)

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

    print(f"Velocidade Filtro: {estado_atualx}, {estado_atualy}, {estado_atualz}")
    cont.append(td - ti)

plt.plot(cont, fx, "-b")
plt.plot(cont, fy, "-g")
plt.plot(cont, fz, "-r")
plt.show()