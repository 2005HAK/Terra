import pixhawk as px
import matplotlib.pyplot as plt

sensors = px.Pixhawk()

accx = []
accy = []
accz = []
cont = []
i = 0

while i < 1000:
    sensors.update_data()

    print(f"Acc: {sensors.get_acc()}")
    print(f"Gyro:{sensors.get_gyro()}")
    print(f"Mag: {sensors.get_mag()}")
    print(f"Vel: {sensors.get_vel()}")

    accx.append(sensors.get_acc()[0])
    accy.append(sensors.get_acc()[1])
    accz.append(sensors.get_acc()[2])
    cont.append(i)
    i += 1

plt.plot(cont, accx, "-b")
plt.plot(cont, accy, "-g")
plt.plot(cont, accz, "-r")
plt.show()