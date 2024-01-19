import time
import RPi.GPIO as GPIO
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)

GPIO.setup(7, GPIO.OUT)

p = GPIO.PWM(7, 50)

p.start(0)
print("Starting 0")
time.sleep(3)

p.ChangeDutyCycle(5.3)
print("Start")
time.sleep(200)