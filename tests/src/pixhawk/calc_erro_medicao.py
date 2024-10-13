import pixhawk as px
import matplotlib.pyplot as plt
import time

sensors = px.Pixhawk()

fx = 0
fy = 0
fz = 0
cont = []
i = 0

while i < 1000:
    sensors.update_data()

    fx += sensors.get_vel()[0]
    fy += sensors.get_vel()[1]
    fz += sensors.get_vel()[2] 

    cont.append(i)
    i += 1

print(fx / i)
print(fy / i)
print(fz / i)