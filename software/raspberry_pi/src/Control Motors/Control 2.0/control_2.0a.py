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

# value for the motor to stop
rest_value = 1500

pi = pigpio.pi()
# 
# while(not pi.connect()):
#     pi = pigpio.pi()

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
    value_micro = value * 4

    return (1500 + value_micro)

def convert_reverse(value):
    value_micro = value * 4
    
    return (1500 - value_micro)

def finish():
    """
    Finish the motors
    """
    for pin in PINS:
        pi.set_PWM_dutycycle(pin, 0)
    pi.stop()

# Test
if __name__ == "__main__":
    motors_control({"DOWN": 80, "UP": 60})