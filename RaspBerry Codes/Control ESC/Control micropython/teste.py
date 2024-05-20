from machine import Pin
import utime

led = Pin(25, Pin.OUT)  # O LED embutido está no pino 25

while True:
    led.value(1)  # Liga o LED embutido
    utime.sleep(0.01)         # Espera 1 segundo
    led.value(0)  # Desliga o LED embutido
    utime.sleep(1)         # Espera 1 segundo
