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

w = int(input("1 - Gráfico, 2 - Valores em tempo real: "))

match(w):
    case 1:
        values = int(input("Quantos dados de amostra? "))
        w = int(input("1 - Aceleração, 2 - Velocidade, 3 - Giroscópio, 4 - Magnetrometro: "))
        match(w):
            case 1: 
                while i < values:
                    px.update_data()
                    velx.append(px.get_acc()[0])
                    vely.append(px.get_acc()[1])
                    velz.append(px.get_acc()[2])
                    cont.append(i)
                    # print(px.current_time - px.old_time)
                    print(i)
                    i += 1
            case 2: 
                while i < values:
                    px.update_data()
                    velx.append(px.get_vel()[0])
                    vely.append(px.get_vel()[1])
                    velz.append(px.get_vel()[2])
                    cont.append(i)
                    # print(px.current_time - px.old_time)
                    print(i)
                    i += 1
            case 3: 
                while i < values:
                    px.update_data()
                    velx.append(px.get_gyro()[0])
                    vely.append(px.get_gyro()[1])
                    velz.append(px.get_gyro()[2])
                    cont.append(i)
                    # print(px.current_time - px.old_time)
                    print(i)
                    i += 1
            case 4: 
                while i < values:
                    px.update_data()
                    velx.append(px.get_mag()[0])
                    vely.append(px.get_mag()[1])
                    velz.append(px.get_mag()[2])
                    cont.append(i)
                    # print(px.current_time - px.old_time)
                    print(i)
                    i += 1

        plt.plot(cont, velx, "-b")
        plt.plot(cont, vely, "-g")
        plt.plot(cont, velz, "-r")

        plt.show()

    case 2:
        w = int(input("1 - Aceleração, 2 - Velocidade, 3 - Giroscópio, 4 - Magnetrometro: "))
        match w:
            case 1:
                while 1:
                    px.update_data()
                    print(f"Aceleração: {px.get_acc()}")    
                    # print(px.current_time - px.old_time)
                    os.system("clear")
            case 2:
                while 1:
                    px.update_data()
                    print(f"Velocidade: {px.get_vel()}")
                    # print(px.current_time - px.old_time)
                    # os.system("clear")
            case 3:
                while 1:
                    px.update_data()
                    print(f"Giroscópio: {px.get_gyro()}")
                    # print(px.current_time - px.old_time)
                    os.system("clear")
            case 4:
                while 1:
                    px.update_data()
                    print(f"Magnetrome: {px.get_mag()}")
                    # print(px.current_time - px.old_time)
                    os.system("clear")
