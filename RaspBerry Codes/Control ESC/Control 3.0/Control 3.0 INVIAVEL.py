"""
Vou tentar utilizar uma aproximação para um polinomio para que a força reflita em um pwm
"""

# Tentativa de usar polinomios de lagrange

"""












INVIÁVEL
















"""
import matplotlib.pyplot as plt
import numpy as np

X = [1000, 1020, 1040, 1060, 1080, 1100, 1120, 1140, 1160, 1180, 1200, 1220, 1240, 1260, 1280, 1300, 1320, 1340, 1360, 1380, 1400, 1420, 1440, 1460, 1480, 1500, 1520, 1540, 1560, 1580, 1600, 1620, 1640, 1660, 1680, 1700, 1720, 1740, 1760, 1780, 1800, 1820, 1840, 1860, 1880, 1900, 1920, 1940, 1960, 1980, 2000]
Y = [3.8, 3.7 , 3.5, 3.2, 3.0, 2.9, 2.76, 2.56, 2.34, 2.14, 1.95, 1.73, 1.53, 1.37, 1.19, 1.02, 0.87, 0.72, 0.57, 0.44, 0.32, 0.2, 0.11, 0.04, 0, 0, 0, 0.04, 0.13, 0.25, 0.39, 0.55, 0.71, 0.89, 1.08, 1.28, 1.48, 1.71, 1.92, 2.18, 2.46, 2.71, 3.01, 3.26, 3.5, 3.71, 3.9, 4.1, 4.25, 4.4, 4.55]

def pol_lagrange(X, Y, x):
    L = []
    tam = len(X)
    for i in range(tam):
        prod = 1
        for j in range(tam):
            if j != i:
                prod *= (x - X[j]) / (X[i] - X[j])
        L.append(prod)

    y = 0      
    for i in range(tam):
        y += Y[i] * L[i]
    
    return y

def show_grafic(x, y):
    plt.plot(x, y)
    plt.show()

if __name__ == "__main__":
    x = 1100
    y = 0
    while x < 1900:
        y = pol_lagrange(X, Y, x)
        
        print(f"PWM: {x}, KGf: {y}")

        x += 1

    
