pwm_values = [1100, 1120, 1140, 1160, 1180, 1200, 1220, 1240, 1260, 1280, 1300, 
              1320, 1340, 1360, 1380, 1400, 1420, 1440, 1460, 1480, 1500, 1520, 
              1540, 1560, 1580, 1600, 1620, 1640, 1660, 1680, 1700, 1720, 1740, 
              1760, 1780, 1800, 1820, 1840, 1860, 1880, 1900]

kgf_values = [2.9, 2.76, 2.56, 2.34, 2.14, 1.95, 1.73, 1.53, 1.37, 1.19, 1.02, 
              0.87, 0.72, 0.57, 0.44, 0.32, 0.2, 0.11, 0.04, 0, 0, 0, 0.04, 
              0.13, 0.25, 0.39, 0.55, 0.71, 0.89, 1.08, 1.28, 1.48, 1.71, 1.92, 
              2.18, 2.46, 2.71, 3.01, 3.26, 3.5, 3.71]

def convert_forward(value):
    if(value > 10):
        kgf = value * 0.0371

        i = 20

        while kgf_values[i] < kgf:
            i += 1
        
        power = int((((pwm_values[i - 1] - 1500) * value) / (26.954 * kgf_values[i - 1])) + 1498)

        print(f"Kgf: {kgf}, PWM: {power}")
    else:
        print(f"Kgf: 0, PWM: {1500}")

def convert_reverse(value):
    if(value > 5):
        kgf = value * 0.029

        i = 20

        while kgf_values[i] < kgf:
            i -= 1
        
        power = int((((pwm_values[i + 1] - 1500) * value) / (34.483 * kgf_values[i + 1])) + 1500)

        print(f"Kgf: {kgf}, PWM: {power}")
    else:
        print(f"10: {1500}")

if __name__ == "__main__":
    for i in range(0,101, 5):
        convert_reverse(i)
    for i in range(0,101, 5):
        convert_forward(i)