import interpolacao as ipo

FREQUENCY = 50

pwm_values = [1100, 1120, 1140, 1160, 1180, 1200, 1220, 1240, 1260, 1280, 1300, 1320, 1340, 1360, 1380, 1400, 1420, 1440, 1460, 1480, 1500, 1520, 1540, 1560, 1580, 1600, 1620, 1640, 1660, 1680, 1700, 1720, 1740, 1760, 1780, 1800, 1820, 1840, 1860, 1880, 1900]

kgf_values = [2.9, 2.76, 2.56, 2.34, 2.14, 1.95, 1.73, 1.53, 1.37, 1.19, 1.02, 0.87, 0.72, 0.57, 0.44, 0.32, 0.2, 0.11, 0.04, 0, 0, 0, 0.04, 0.13, 0.25, 0.39, 0.55, 0.71, 0.89, 1.08, 1.28, 1.48, 1.71, 1.92, 2.18, 2.46, 2.71, 3.01, 3.26, 3.5, 3.71]

coef = ipo.spline(pwm_values, kgf_values)

def convert_forward(value):
    kgf = value * 0.0371
    i = 20

    while kgf_values[i] < kgf:
        i += 1
    if i != 20:
        i -= 1

    x = []

    for j in range(21):
        x.append(pwm_values[i] + j)

    y = ipo.calc_spline(pwm_values[i], x, coef[i])

    i = 0

    while y[i] < kgf:
        i += 1

    return x[i]

def convert_reverse(value):
    kgf = value * 0.0289

    i = 20

    while kgf_values[i] < kgf:
        i -= 1
    
    x = []

    for j in range(21):
        x.append(pwm_values[i] + j)

    y = ipo.calc_spline(pwm_values[i], x, coef[i])

    i = 0
    while y[i] > kgf:
        i += 1

    return x[i]

# Test
if __name__ == "__main__":
    # for i in range(101):
    #     print(convert_reverse(i), end=", ")
    # for i in range(101):
    #     print(convert_forward(i), end=", ")

    for i in range(101):
        print(f"{i}: {convert_reverse(i)}")
    for i in range(101):
        print(f"{i}: {convert_forward(i)}")