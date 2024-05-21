from machine import Pin, PWM
from utime import sleep

FREQ = 400

motor1 = PWM(Pin(25), freq = FREQ)

# Freq 400Hz: duty de 44% a 76%
# Freq 100Hz: duty de 11% a 19%

def init_esc():
    print("Inicializando . . .")
    value_init = int(98.3144 * FREQ) # Equivalente a 1500 us
    print(int((15.2587890625 * value_init) / FREQ)) 
    motor1.duty_u16(value_init)
    # sleep(7)

def control(percent_value):
    """
    Transforma um valor percentual no equivaloente para o duty cicle de 0 a 65536

    Input:
    - percent_value: valor percentual de 0 a 100
    """
    duty = int(percent_value * 655.36)
    motor1.duty_u16(duty)

def motor_control(percent_value):
    """
    Recebe um valor de 0 a 100 % e este valor é convertido para o valor correspondente de duty cicle dentro do intervalo de 1100 a 1900 us
    
    Input:
    - percent_value: valor entre 0 e 100 % (0 a 49 % corresponde ao reverso e 50 a 100 % corresponde para frente)
    """

    duty = int(FREQ * (0.524288 * percent_value + 72.1)) # Encontra o valor do duty cicle correspontende 
    print(int((15.2587890625 * duty) / FREQ)) # Exibe a frequencia usada
    motor1.duty_u16(duty)

def convert_forward(percent_value):
    duty = int(FREQ * (0.262144 * percent_value + 98.35)) # Encontra o valor do duty cicle correspontende 
    print(int((15.2587890625 * duty) / FREQ)) # Exibe a frequencia usada
    motor1.duty_u16(duty)

def convert_reverse(percent_value):
    pass

def finalize_esc():
    print("Finalizando . . .")
    motor1.duty_u16(0) # Zera o PWM do pino do motor

# Test
init_esc()

for i in range(101):
    convert_forward(i)
    sleep(0.01)
# for i in range(101):
#     motor_control(i)
#     sleep(0.01)

# motor_control(0)
# motor_control(100)

finalize_esc()