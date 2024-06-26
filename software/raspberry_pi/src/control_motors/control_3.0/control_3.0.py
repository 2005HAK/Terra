import pigpio
import time
import interpolation as ipo

FREQUENCY = 50

# Pins motors
PINS = [1, 2, 3, 4, 5, 6]
# 0 - Front left 
# 1 - Front right
# 2 - Middle right
# 3 - Middle left
# 4 - Back right
# 5 - Back left 

# value for the motor to stop
rest_value = 1500

pwm_values = [1100, 1120, 1140, 1160, 1180, 1200, 1220, 1240, 1260, 1280, 1300, 1320, 1340, 1360, 1380, 1400, 1420, 1440, 1460, 1480, 1500, 1520, 1540, 1560, 1580, 1600, 1620, 1640, 1660, 1680, 1700, 1720, 1740, 1760, 1780, 1800, 1820, 1840, 1860, 1880, 1900]

kgf_values = [2.9, 2.76, 2.56, 2.34, 2.14, 1.95, 1.73, 1.53, 1.37, 1.19, 1.02, 0.87, 0.72, 0.57, 0.44, 0.32, 0.2, 0.11, 0.04, 0, 0, 0, 0.04, 0.13, 0.25, 0.39, 0.55, 0.71, 0.89, 1.08, 1.28, 1.48, 1.71, 1.92, 2.18, 2.46, 2.71, 3.01, 3.26, 3.5, 3.71]

coef = ipo.spline(pwm_values, kgf_values)

pi = pigpio.pi()
# # 
# # while(not pi.connect()):
# #     pi = pigpio.pi()

def inicialize_pins():
    for pin in PINS:
        pi.set_mode(pin, pigpio.OUTPUT)
        pi.set_PWM_frequency(pin, FREQUENCY)
        pi.set_servo_pulsewidth(pin, rest_value)
    time.sleep(7)

def motors_control(actions):
    """
    Function that takes an argument with movement instruction and decides how the motors will be activated

    Parameters:

    - action       : action that should be execute 

    actions:
    - "UP"         : move the AUV up, turning on the motors 3 and 6 in the forward direction.
    - "DOWN"       : move the AUV down, turning on the motors 3 and 6 in the reverse direction.
    - "FRONT"      : move the AUV front, turning on the motors 1 and 2 in the reverse direction and the motors 4 and 5 in the forward direction.
    - "BACK"       : move the AUV back, turning on the motors 1 and 2 in the forward direction and the motors 4 and 5 in the reverse direction.
    - "RIGHT"      : move the AUV right, turning on the motors 2 and 4 in the forward direction and the motors 1 and 5 in the reverse direction.
    - "LEFT"       : move the AUV left, turning on the motors 1 and 5 in the forward direction and the motors 2 and 4 in the reverse direction.
    - "TURN RIGHT" : turn the AUV right, turning on the motors 2 and 5 in the forward direction and the motors 1 and 4 in the reverse direction.
    - "TURN LEFT"  : turn the AUV left, turning on the motors 1 and 4 in the forward direction and the motors 2 and 5 in the reverse direction.

    Return:
    None
    """

    for action, value in actions.items():
        forward_value = convert_forward(value)
        reverse_value = convert_reverse(value)

        if action == "UP":
            pi.set_servo_pulsewidth(PINS[2], forward_value)
            pi.set_servo_pulsewidth(PINS[5], forward_value)
        elif action == "DOWN":
            pi.set_servo_pulsewidth(PINS[2], reverse_value)
            pi.set_servo_pulsewidth(PINS[5], reverse_value)

        if action == "FRONT":
            pi.set_servo_pulsewidth(PINS[0], reverse_value)
            pi.set_servo_pulsewidth(PINS[1], reverse_value)
            pi.set_servo_pulsewidth(PINS[3], forward_value)
            pi.set_servo_pulsewidth(PINS[4], forward_value)
        if action == "BACK":
            pi.set_servo_pulsewidth(PINS[0], forward_value)
            pi.set_servo_pulsewidth(PINS[1], forward_value)
            pi.set_servo_pulsewidth(PINS[3], reverse_value)
            pi.set_servo_pulsewidth(PINS[4], reverse_value)
        if action == "RIGHT":
            pi.set_servo_pulsewidth(PINS[0], reverse_value)
            pi.set_servo_pulsewidth(PINS[1], forward_value)
            pi.set_servo_pulsewidth(PINS[3], forward_value)
            pi.set_servo_pulsewidth(PINS[4], reverse_value)
        if action == "LEFT":
            pi.set_servo_pulsewidth(PINS[0], forward_value)
            pi.set_servo_pulsewidth(PINS[1], reverse_value)
            pi.set_servo_pulsewidth(PINS[3], reverse_value)
            pi.set_servo_pulsewidth(PINS[4], forward_value)
        if action == "TURN RIGHT":
            pi.set_servo_pulsewidth(PINS[0], reverse_value)
            pi.set_servo_pulsewidth(PINS[1], forward_value)
            pi.set_servo_pulsewidth(PINS[3], reverse_value)
            pi.set_servo_pulsewidth(PINS[4], forward_value)
        if action == "TURN LEFT":
            pi.set_servo_pulsewidth(PINS[0], forward_value)
            pi.set_servo_pulsewidth(PINS[1], reverse_value)
            pi.set_servo_pulsewidth(PINS[3], forward_value)
            pi.set_servo_pulsewidth(PINS[4], reverse_value)
        if action == "STOP":
            pi.set_servo_pulsewidth(PINS[0], rest_value)
            pi.set_servo_pulsewidth(PINS[1], rest_value)
            pi.set_servo_pulsewidth(PINS[2], rest_value)
            pi.set_servo_pulsewidth(PINS[3], rest_value)
            pi.set_servo_pulsewidth(PINS[4], rest_value)
            pi.set_servo_pulsewidth(PINS[5], rest_value)

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

def finish():
    """
    Finish the motors
    """
    for pin in PINS:
        pi.set_PWM_dutycycle(pin, 0)
    pi.stop()

# Test
if __name__ == "__main__":
    while True:
        motors_control({"DOWN": 80, "UP": 60})
        time.sleep(1)

    # for i in range(100, -1, -1):
    #     print(convert_reverse(i))
    # for i in range(101):
    #     print(convert_forward(i))