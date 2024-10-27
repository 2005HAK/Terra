import pixhawk
import os
import time
import matplotlib.pyplot as plt

px = pixhawk.Pixhawk()
velx = []
vely = []
velz = []
cont = []
i = 0

# while i < 1000:
#     px.update_data()
#     velx.append(px.get_vel()[0])
#     vely.append(px.get_vel()[1])
#     velz.append(px.get_vel()[2])
#     cont.append(i)
#     print(px.current_time - px.old_time)
#     i += 1

# plt.plot(cont, velx, "-b")
# plt.plot(cont, vely, "-g")
# plt.plot(cont, velz, "-r")

# plt.show()

while 1:
    px.update_data()
    # print(f"Aceleração: {px.get_acc()}")
    print(f"Velocidade: {px.get_vel()}")
    # print(f"Giroscópio: {px.get_gyro()}")
    # print(f"Magnetrome: {px.get_mag()}")
    os.system("cls")
    # print(px.current_time - px.old_time)
