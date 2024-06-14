from machine import Pin, PWM
from utime import sleep

FREQ = 200 # 50 - 400 Hz

PINS = [16, 17, 18, 19, 20, 21]
# 0 - Front left 
# 1 - Front right
# 2 - Middle right
# 3 - Middle left
# 4 - Back right
# 5 - Back left 

motores = []

rest_value = int(98.3144 * FREQ) # Equivalente a 1500 us

def init_esc():
    """
    Inicializa o ESC, passa um PWM de 1500us e mantem por 7 segundos, que é o tempo que o ESC 
    precisa, para então o motor estar pronto para ser usado
    """
    
    print("Starting . . .")

    for i in PINS:
        motor = PWM(Pin(i), freq = FREQ)
        motor.duty_u16(rest_value)
        motores.append(motor)
    
    print(int((15.2587890625 * rest_value) / FREQ)) # shows the PWM used

    sleep(7)

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
            motores[2].duty_u16(forward_value)
            motores[5].duty_u16(forward_value)
        elif action == "DOWN":
            motores[2].duty_u16(reverse_value)
            motores[5].duty_u16(reverse_value)

        if action == "FRONT":
            motores[0].duty_u16(reverse_value)
            motores[1].duty_u16(reverse_value)
            motores[3].duty_u16(forward_value)
            motores[4].duty_u16(forward_value)
        if action == "BACK":
            motores[0].duty_u16(forward_value)
            motores[1].duty_u16(forward_value)
            motores[3].duty_u16(reverse_value)
            motores[4].duty_u16(reverse_value)
        if action == "RIGHT":
            motores[0].duty_u16(reverse_value)
            motores[1].duty_u16(forward_value)
            motores[3].duty_u16(forward_value)
            motores[4].duty_u16(reverse_value)
        if action == "LEFT":
            motores[0].duty_u16(forward_value)
            motores[1].duty_u16(reverse_value)
            motores[3].duty_u16(reverse_value)
            motores[4].duty_u16(forward_value)
        if action == "TURN RIGHT":
            motores[0].duty_u16(reverse_value)
            motores[1].duty_u16(forward_value)
            motores[3].duty_u16(reverse_value)
            motores[4].duty_u16(forward_value)
        if action == "TURN LEFT":
            motores[0].duty_u16(forward_value)
            motores[1].duty_u16(reverse_value)
            motores[3].duty_u16(forward_value)
            motores[4].duty_u16(reverse_value)
        if action == "STOP":
            motores[0].duty_u16(rest_value)
            motores[1].duty_u16(rest_value)
            motores[2].duty_u16(rest_value)
            motores[3].duty_u16(rest_value)
            motores[4].duty_u16(rest_value)
            motores[5].duty_u16(rest_value)

def convert_forward(percent_value):
    """
    Recebe um valor de 0% a 100% e converte em uma valor de PWM equivalente entre 1500us e 1900us para que o 
    motor "gire para frente"

    Input:
    - percent_value: valor entre 0 e 100 %
    """

    duty = int(FREQ * (0.262144 * percent_value + 98.35)) # Encontra o valor do duty cicle correspontende 
    print(int((15.2587890625 * duty) / FREQ)) # Exibe a frequencia usada
    return duty

def convert_reverse(percent_value):
    """
    Recebe um valor de 0% a 100% e converte em uma valor de PWM equivalente entre 1500us e 1100us para que o 
    motor "gire para trás"

    Input:
    - percent_value: valor entre 0 e 100 %
    """

    duty = int(FREQ * (98.35 - 0.262144 * percent_value)) # Encontra o valor do duty cicle correspontende 
    print(int((15.2587890625 * duty) / FREQ)) # Exibe a frequencia usada
    return duty

def finalize_esc():
    """
    Finaliza a execução do ESC, passa um PWM de 1500us para que o motor pare
    """
    
    print("Finishing . . .")

    for motor in motores:
        motor.duty_u16(rest_value)
        motor.duty_u16(0)

# Test
if __name__ == "__main__":
    """
    Algoritmo para testar o motor usando micropython
    """

    init_esc()

    # NÃO PASSAR DE 30% COM A FONTE DE 3A

    motors_control({"UP": 30})
    sleep(100)
    motors_control({"FRONT": 40})
    sleep(30)
    motors_control({"LEFT": 20})
    sleep(100)

    finalize_esc()