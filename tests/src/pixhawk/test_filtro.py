import pixhawk as px
from pymavlink import mavutil
import matplotlib.pyplot as plt

connection = mavutil.mavlink_connection('/dev/ttyACM0', baud = 115200)

ruido = .2
erro_medicao = .2

estimativa_inicial = 0
variancia_estimativa = .5

variancia_estimativa_estrapolada = variancia_estimativa * variancia_estimativa + ruido

i = 0
valores = [estimativa_inicial]
realx = [0]
cont = [i]

while i < 100:
    msg = connection.recv_match(type='LOCAL_POSITION_NED', blocking=True)

    if not msg:
        continue
    
    ganho_kalman = variancia_estimativa_estrapolada / (variancia_estimativa_estrapolada + erro_medicao)
    estado_atual = estimativa_inicial + ganho_kalman * (msg.vx - estimativa_inicial)
    variancia_estado_atual = (1 - ganho_kalman) * variancia_estimativa_estrapolada

    estimativa_inicial = estado_atual
    variancia_estimativa_estrapolada = variancia_estado_atual + ruido

    print(estimativa_inicial)

    valores.append(estimativa_inicial)
    realx.append(msg.vx)
    print(msg.vx)
    i += 1
    cont.append(i)


plt.plot(cont, valores, "-r")
plt.plot(cont, realx, "-g")

plt.show()