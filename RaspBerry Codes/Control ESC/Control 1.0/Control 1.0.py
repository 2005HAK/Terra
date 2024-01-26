import pigpio
import time

# Pins motors
PINS = [1, 2, 3, 4, 5, 6]
# 1 - Front left 
# 2 - Front right
# 3 - Middle right
# 4 - Middle left
# 5 - Back right
# 6 - Back left 

pi = pigpio.pi()

for pin in PINS:
    pi.set_mode(pin, pigpio.OUTPUT)
    pi.set_PWM_frequency(pin, 50)
    pi.set_servo_pulsewidth(pin, 1500)
time.sleep(2)

def motors_control(action):
    """
    Function that takes an argument with movement instruction and decides how the motors will be activated
    """
    if action == "UP":
        pi.set_servo_pulsewidth(3, 1900)
        pi.set_servo_pulsewidth(6, 1900)
    elif action == "DOWN":
        pi.set_servo_pulsewidth(3, 1100)
        pi.set_servo_pulsewidth(6, 1100)
    else:
        pi.set_servo_pulsewidth(3, 1500)
        pi.set_servo_pulsewidth(6, 1500)
    if action == "FRONT":
        pi.set_servo_pulsewidth(1, 1100)
        pi.set_servo_pulsewidth(2, 1100)
        pi.set_servo_pulsewidth(4, 1900)
        pi.set_servo_pulsewidth(5, 1900)
    else:
        pi.set_servo_pulsewidth(1, 1500)
        pi.set_servo_pulsewidth(2, 1500)
        pi.set_servo_pulsewidth(4, 1500)
        pi.set_servo_pulsewidth(5, 1500)
    if action == "BACK":
        pi.set_servo_pulsewidth(1, 1900)
        pi.set_servo_pulsewidth(2, 1900)
        pi.set_servo_pulsewidth(4, 1100)
        pi.set_servo_pulsewidth(5, 1100)
    else:
        pi.set_servo_pulsewidth(1, 1500)
        pi.set_servo_pulsewidth(2, 1500)
        pi.set_servo_pulsewidth(4, 1500)
        pi.set_servo_pulsewidth(5, 1500)
    if action == "RIGHT":
        pi.set_servo_pulsewidth(1, 1100)
        pi.set_servo_pulsewidth(2, 1900)
        pi.set_servo_pulsewidth(4, 1900)
        pi.set_servo_pulsewidth(5, 1100)
    else:
        pi.set_servo_pulsewidth(1, 1500)
        pi.set_servo_pulsewidth(2, 1500)
        pi.set_servo_pulsewidth(4, 1500)
        pi.set_servo_pulsewidth(5, 1500)
    if action == "LEFT":
        pi.set_servo_pulsewidth(1, 1900)
        pi.set_servo_pulsewidth(2, 1100)
        pi.set_servo_pulsewidth(4, 1100)
        pi.set_servo_pulsewidth(5, 1900)
    else:
        pi.set_servo_pulsewidth(1, 1500)
        pi.set_servo_pulsewidth(2, 1500)
        pi.set_servo_pulsewidth(4, 1500)
        pi.set_servo_pulsewidth(5, 1500)
    if action == "TURN RIGHT":
        pi.set_servo_pulsewidth(1, 1100)
        pi.set_servo_pulsewidth(2, 1900)
        pi.set_servo_pulsewidth(4, 1100)
        pi.set_servo_pulsewidth(5, 1900)
    else:
        pi.set_servo_pulsewidth(1, 1500)
        pi.set_servo_pulsewidth(2, 1500)
        pi.set_servo_pulsewidth(4, 1500)
        pi.set_servo_pulsewidth(5, 1500)
    if action == "TURN LEFT":
        pi.set_servo_pulsewidth(1, 1900)
        pi.set_servo_pulsewidth(2, 1100)
        pi.set_servo_pulsewidth(4, 1900)
        pi.set_servo_pulsewidth(5, 1100)
    else:
        pi.set_servo_pulsewidth(1, 1500)
        pi.set_servo_pulsewidth(2, 1500)
        pi.set_servo_pulsewidth(4, 1500)
        pi.set_servo_pulsewidth(5, 1500)
    

def finish():
    """
    Finish the motors
    """
    for pin in PINS:
        pi.set_PWM_dutycycle(pin, 0)
    pi.stop()