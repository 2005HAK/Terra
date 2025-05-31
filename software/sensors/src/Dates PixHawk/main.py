from pymavlink import mavutil
import time

def main():
    """
    Variaveis:
    dados - [xacc, yacc, zacc, xgyro, ygyro, zgyro, xmag, ymag, zmag]
    """

    while True:
        try:
            print("Coletando dados...")
            dados = read_dates()
        except:
            print("Erro ao coletar dados")
        
        print(dados[0])
        print(dados[1])
        print(dados[2])
        print(dados[3])
        print(dados[4])
        print(dados[5])
        print(dados[6])
        print(dados[7])
        print(dados[8])

        time.sleep(2)

def read_dates():
    dados = [0, 0, 0, 0, 0, 0, 0, 0, 0]
    validoSIMSTATE = False
    validoSCALEDIMU2 = False

    master = mavutil.mavlink_connection('127.0.0.1:14550')
    
    while(not validoSIMSTATE or not validoSCALEDIMU2):
        msg = master.recv_match()

        if not msg:
            continue
        elif msg.get_type() == 'SIMSTATE':
            dados[0] = float(msg.xacc)
            dados[1] = float(msg.yacc)
            dados[2] = float(msg.zacc)
            dados[3] = float(msg.xgyro)
            dados[4] = float(msg.ygyro)
            dados[5] = float(msg.zgyro)
            validoSIMSTATE = True
        elif msg.get_type() == 'SCALED_IMU2':
            dados[6] = float(msg.xmag)
            dados[7] = float(msg.ymag)
            dados[8] = float(msg.zmag)
            validoSCALEDIMU2 = True

    return dados

if(__name__ == "__main__"):
    main()