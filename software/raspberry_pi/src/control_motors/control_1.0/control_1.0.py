import pigpio
import time

FREQUENCY = 50

# Pins motors
PINS = [1, 2, 3, 4, 5, 6]
# 0 - Front left 
# 1 - Front right
# 2 - Middle right
# 3 - Middle left
# 4 - Back right
# 5 - Back left 

pi = pigpio.pi()
# 
# while(not pi.connect()):
#     pi = pigpio.pi()

def inicialize_pins():
    for pin in PINS:
        pi.set_mode(pin, pigpio.OUTPUT)
        pi.set_PWM_frequency(pin, FREQUENCY)
        pi.set_servo_pulsewidth(pin, 1500)
    time.sleep(7)

def motors_control(action):
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

    if action == "UP":
        pi.set_servo_pulsewidth(PINS[2], 1900)
        pi.set_servo_pulsewidth(PINS[5], 1900)
    elif action == "DOWN":
        pi.set_servo_pulsewidth(PINS[2], 1100)
        pi.set_servo_pulsewidth(PINS[5], 1100)
    else:
        pi.set_servo_pulsewidth(PINS[2], 1500)
        pi.set_servo_pulsewidth(PINS[5], 1500)
    if action == "FRONT":
        pi.set_servo_pulsewidth(PINS[0], 1100)
        pi.set_servo_pulsewidth(PINS[1], 1100)
        pi.set_servo_pulsewidth(PINS[3], 1900)
        pi.set_servo_pulsewidth(PINS[4], 1900)
    else:
        pi.set_servo_pulsewidth(PINS[0], 1500)
        pi.set_servo_pulsewidth(PINS[1], 1500)
        pi.set_servo_pulsewidth(PINS[3], 1500)
        pi.set_servo_pulsewidth(PINS[4], 1500)
    if action == "BACK":
        pi.set_servo_pulsewidth(PINS[0], 1900)
        pi.set_servo_pulsewidth(PINS[1], 1900)
        pi.set_servo_pulsewidth(PINS[3], 1100)
        pi.set_servo_pulsewidth(PINS[4], 1100)
    else:
        pi.set_servo_pulsewidth(PINS[0], 1500)
        pi.set_servo_pulsewidth(PINS[1], 1500)
        pi.set_servo_pulsewidth(PINS[3], 1500)
        pi.set_servo_pulsewidth(PINS[4], 1500)
    if action == "RIGHT":
        pi.set_servo_pulsewidth(PINS[0], 1100)
        pi.set_servo_pulsewidth(PINS[1], 1900)
        pi.set_servo_pulsewidth(PINS[3], 1900)
        pi.set_servo_pulsewidth(PINS[4], 1100)
    else:
        pi.set_servo_pulsewidth(PINS[0], 1500)
        pi.set_servo_pulsewidth(PINS[1], 1500)
        pi.set_servo_pulsewidth(PINS[3], 1500)
        pi.set_servo_pulsewidth(PINS[4], 1500)
    if action == "LEFT":
        pi.set_servo_pulsewidth(PINS[0], 1900)
        pi.set_servo_pulsewidth(PINS[1], 1100)
        pi.set_servo_pulsewidth(PINS[3], 1100)
        pi.set_servo_pulsewidth(PINS[4], 1900)
    else:
        pi.set_servo_pulsewidth(PINS[0], 1500)
        pi.set_servo_pulsewidth(PINS[1], 1500)
        pi.set_servo_pulsewidth(PINS[3], 1500)
        pi.set_servo_pulsewidth(PINS[4], 1500)
    if action == "TURN RIGHT":
        pi.set_servo_pulsewidth(PINS[0], 1100)
        pi.set_servo_pulsewidth(PINS[1], 1900)
        pi.set_servo_pulsewidth(PINS[3], 1100)
        pi.set_servo_pulsewidth(PINS[4], 1900)
    else:
        pi.set_servo_pulsewidth(PINS[0], 1500)
        pi.set_servo_pulsewidth(PINS[1], 1500)
        pi.set_servo_pulsewidth(PINS[3], 1500)
        pi.set_servo_pulsewidth(PINS[4], 1500)
    if action == "TURN LEFT":
        pi.set_servo_pulsewidth(PINS[0], 1900)
        pi.set_servo_pulsewidth(PINS[1], 1100)
        pi.set_servo_pulsewidth(PINS[3], 1900)
        pi.set_servo_pulsewidth(PINS[4], 1100)
    else:
        pi.set_servo_pulsewidth(PINS[0], 1500)
        pi.set_servo_pulsewidth(PINS[1], 1500)
        pi.set_servo_pulsewidth(PINS[3], 1500)
        pi.set_servo_pulsewidth(PINS[4], 1500) 

def finish():
    """
    Finish the motors
    """
    for pin in PINS:
        pi.set_PWM_dutycycle(pin, 0)
    pi.stop()

if __name__ == "__main__":
    inicialize_pins()