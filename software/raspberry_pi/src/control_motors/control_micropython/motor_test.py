from machine import Pin, PWM
from utime import sleep

FREQ = 200
PIN = 20
PIN2 = 21


motor1 = PWM(Pin(PIN), freq = FREQ)
motor2 = PWM(Pin(PIN2), freq = FREQ)

# Freq 400Hz: duty de 44% a 76%
# Freq 100Hz: duty de 11% a 19%

def init_esc():
    """
    Inicializa o ESC, passa um PWM de 1500us e mantem por 7 segundos, que é o tempo que o ESC 
    precisa, para então o motor estar pronto para ser usado
    """
    
    print("Starting . . .")
    value_init = int(98.3144 * FREQ) # Equivalente a 1500 us
    print(int((15.2587890625 * value_init) / FREQ)) 
    motor1.duty_u16(value_init)
    motor2.duty_u16(value_init)
    sleep(7)

def motor_control(percent_value):
    """
    Recebe um valor de 0 a 100 % e este valor é convertido para o valor correspondente de duty cicle dentro do 
    intervalo de 1100 a 1900 us
    
    Input:
    - percent_value: valor entre 0 e 100 % (0 a 49 % corresponde ao reverso e 50 a 100 % corresponde para frente)
    """

    duty = int(FREQ * (0.524288 * percent_value + 72.1)) # Encontra o valor do duty cicle correspontende 
    print(int((15.2587890625 * duty) / FREQ)) # Exibe a frequencia usada
    motor1.duty_u16(duty)

def convert_forward1(percent_value):
    """
    Recebe um valor de 0% a 100% e converte em uma valor de PWM equivalente entre 1500us e 1900us para que o 
    motor "gire para frente"

    Input:
    - percent_value1: valor entre 0 e 100 %
    """

    duty = int(FREQ * (0.262144 * percent_value + 98.35)) # Encontra o valor do duty cicle correspontende 
    print(int((15.2587890625 * duty) / FREQ)) # Exibe a frequencia usada
    motor1.duty_u16(duty)

def convert_forward2(percent_value):
    """
    Recebe um valor de 0% a 100% e converte em uma valor de PWM equivalente entre 1500us e 1900us para que o 
    motor "gire para frente"

    Input:
    - percent_value1: valor entre 0 e 100 %
    """

    duty = int(FREQ * (0.262144 * percent_value + 98.35)) # Encontra o valor do duty cicle correspontende 
    print(int((15.2587890625 * duty) / FREQ)) # Exibe a frequencia usada
    motor2.duty_u16(duty)

def convert_reverse1(percent_value):
    """
    Recebe um valor de 0% a 100% e converte em uma valor de PWM equivalente entre 1500us e 1100us para que o 
    motor "gire para trás"

    Input:
    - percent_value1: valor entre 0 e 100 %
    """

    duty = int(FREQ * (98.35 - 0.262144 * percent_value)) # Encontra o valor do duty cicle correspontende 
    print(int((15.2587890625 * duty) / FREQ)) # Exibe a frequencia usada
    motor1.duty_u16(duty)

def convert_reverse2(percent_value):
    """
    Recebe um valor de 0% a 100% e converte em uma valor de PWM equivalente entre 1500us e 1100us para que o 
    motor "gire para trás"

    Input:
    - percent_value1: valor entre 0 e 100 %
    """

    duty = int(FREQ * (98.35 - 0.262144 * percent_value)) # Encontra o valor do duty cicle correspontende 
    print(int((15.2587890625 * duty) / FREQ)) # Exibe a frequencia usada
    motor2.duty_u16(duty)

def finalize_esc():
    """
    Finaliza a execução do ESC, passa um PWM de 1500us para que o motor pare
    """
    
    print("Finishing . . .")
    value_fin = int(98.3144 * FREQ)
    motor1.duty_u16(value_fin)
    motor2.duty_u16(value_fin)

# Test
if __name__ == "__main__":
    """
    Algoritmo para testar o motor usando micropython

    Esse algoritmo passa valores percentuais para o motor "girar para frente" e para "girar para trás" aguardando
    um tempo para ver o resultado no motor
    """

    init_esc()

    # NÃO PASSAR DE 30% COM A FONTE DE 3A

    while True:
        value1 = int(input("motor 1: "))
        value2 = int(input("motor 2: "))

        if value1 > 0:
            convert_forward1(value1)
        else:
            convert_reverse1(-value1)

        if value2 > 0:
            convert_forward2(value2)
        else:
            convert_reverse2(-value2)