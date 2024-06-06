from machine import Pin, PWM
from utime import sleep

FREQ = 400
PIN = 20

device = PWM(Pin(PIN), freq = FREQ)

def control(percent_value):
    """
    Transforma um valor percentual no equivaloente para o duty cicle de 0 a 65536

    Input:
    - percent_value: valor percentual de 0 a 100
    """
    duty = int(percent_value * 655.36)
    device.duty_u16(duty)

# Test
if __name__ == "__main__":
    """
    Algoritmo para testar o PWM

    Um dispositivo qualquer é ligado a um pino com uma frequencia e o codigo varia o PWM de 0% a 100% e 
    de 100% a 0% em um ciclo indeterminado

    Para sair use CTRL + C
    """

    while True:
        for i in range(101):
            control(i)
            sleep(0.01)
        for i in range(100, -1, -1):
            control(i)
            sleep(0.01)