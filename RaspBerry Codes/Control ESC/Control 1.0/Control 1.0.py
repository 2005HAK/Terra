#!/usr/bin/python
import signal

from pizypwm import *

# Constants for pins and frequency

FREQUENCY_PWM = 333

MR_PIN = 7
ML_PIN = 11
FR_PIN = 12
FL_PIN = 13
BR_PIN = 15
BL_PIN = 16

# Called on process interruption. Set all pins to "Input" default mode.
def endProcess(signalnum = None, handler = None):
    fr.stop()
    fl.stop()
    br.stop()
    bl.stop()
    mr.stop()
    ml.stop()

    GPIO.cleanup()
    exit(0)

# Prepare handlers for process exit
signal.signal(signal.SIGTERM, endProcess)
signal.signal(signal.SIGINT, endProcess)

# Initialize PWM outputs
fr = PiZyPwm(FREQUENCY_PWM, MR_PIN, GPIO.BOARD)
fl = PiZyPwm(FREQUENCY_PWM, ML_PIN, GPIO.BOARD)
br = PiZyPwm(FREQUENCY_PWM, FR_PIN, GPIO.BOARD)
bl = PiZyPwm(FREQUENCY_PWM, FL_PIN, GPIO.BOARD)
mr = PiZyPwm(FREQUENCY_PWM, BR_PIN, GPIO.BOARD)
ml = PiZyPwm(FREQUENCY_PWM, BL_PIN, GPIO.BOARD)

# Start PWM output
fr.start(50)
fl.start(50)
br.start(50)
bl.start(50)
mr.start(50)
ml.start(50)

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