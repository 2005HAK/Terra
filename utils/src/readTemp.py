from psutil import sensors_temperatures
from time import sleep, time
import matplotlib.pyplot as plt
import os

op = int(input("1 - Temperatura em tempo real, 2 - Grafico da temperatura "))

match(op):
	case 1:
		while 1:
			print(sensors_temperatures()["cpu_thermal"][0].current)
			sleep(.2)
			os.system("clear")
	case 2:
		interv = input("Qual o tempo de amostragem? ")
		time_init = time()
		temps = []
		cont = []
		i = 0

		while time() - time_init < int(interv):
			temps.append(sensors_temperatures()["cpu_thermal"][0].current)
			cont.append(i)
			i += 1
			sleep(.2)

		plt.plot(cont, temps, "-b")
		plt.show()
