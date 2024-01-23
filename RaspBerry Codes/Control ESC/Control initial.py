"""
Código de controle dos motores

Implementação básica assumindo que existem 6 pinos com PWM nativo (a principio só existem 4)
"""

import RPi.GPIO as GPIO

# Constants for pins and frequency

FREQUENCY_PWM = 333

MR_PIN = 1
ML_PIN = 2
FR_PIN = 3
FL_PIN = 4
BR_PIN = 5
BL_PIN = 6

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)

for pin in [MR_PIN, ML_PIN, FR_PIN, FL_PIN, BR_PIN, BL_PIN]:
    GPIO.setup(pin, GPIO.OUT)

mr = GPIO.PWM(MR_PIN, FREQUENCY_PWM)
ml = GPIO.PWM(ML_PIN, FREQUENCY_PWM)
fr = GPIO.PWM(FR_PIN, FREQUENCY_PWM)
fl = GPIO.PWM(FL_PIN, FREQUENCY_PWM)
br = GPIO.PWM(BR_PIN, FREQUENCY_PWM)
bl = GPIO.PWM(BL_PIN, FREQUENCY_PWM)

mr.start(50)
ml.start(50)
fr.start(50)
fl.start(50)
br.start(50)
bl.start(50)

def motors_control(action):
    
    ## actions = action.split(" + ")

    ## for act in actions:
    if action == "UP":
        mr.ChangeDutyCycle(63)
        ml.ChangeDutyCycle(63)
    elif action == "DOWN":
        mr.ChangeDutyCycle(33)
        ml.ChangeDutyCycle(33)
    else:
        mr.ChangeDutyCycle(50)
        ml.ChangeDutyCycle(50)
    if action == "FRONT":
        br.ChangeDutyCycle(63)
        bl.ChangeDutyCycle(63)
    else:
        br.ChangeDutyCycle(50)
        bl.ChangeDutyCycle(50)
    if action == "BACK":
        fr.ChangeDutyCycle(63)
        fl.ChangeDutyCycle(63)
    else:
        fr.ChangeDutyCycle(50)
        fl.ChangeDutyCycle(50)
    if action == "RIGHT":
        fr.ChangeDutyCycle(63)
        br.ChangeDutyCycle(63)
    else:
        fr.ChangeDutyCycle(50)
        br.ChangeDutyCycle(50)
    if action == "LEFT":
        fl.ChangeDutyCycle(63)
        bl.ChangeDutyCycle(63)
    else:
        fl.ChangeDutyCycle(50)
        bl.ChangeDutyCycle(50)
    if action == "TURN RIGHT":
        fr.ChangeDutyCycle(63)
        bl.ChangeDutyCycle(63)
    else:
        fr.ChangeDutyCycle(50)
        bl.ChangeDutyCycle(50)
    if action == "TURN LEFT":
        fl.ChangeDutyCycle(63)
        br.ChangeDutyCycle(63)
    else:
        fl.ChangeDutyCycle(50)
        br.ChangeDutyCycle(50)
    
if (__name__ == "__main__"):
    motors_control("UP")