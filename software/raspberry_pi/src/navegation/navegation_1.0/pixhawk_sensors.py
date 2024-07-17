from pymavlink import mavutil

master = mavutil.mavlink_connection('127.0.0.1:14550')

def get_data():
    """
    pega os dados de aceleração nos eixos x, y, z, os valores do giroscopio nos eixos x, y, z e os valores do magnetrometro
    xacc: [m/s²]
    yacc: [m/s²]
    zacc: [m/s²]
    xgyro: [rad/s]
    ygyro: [rad/s]
    zgyro: [rad/s]
    xmag: [mgauss]
    ymag: [mgauss]
    zmag: [mgauss]
    """
    
    sensors_data = []

    while True:
        msg = master.recv_match()

        if not msg:
            continue
        if msg.get_type() == 'SIMSTATE':
            sensors_data = [msg.xacc, msg.yacc, msg.zacc, msg.xgyro, msg.ygyro, msg.zgyro]
        if msg.get_type() == 'SCALED_IMU2':
            sensors_data.append(msg.xmag)
            sensors_data.append(msg.ymag)
            sensors_data.append(msg.zmag)
        if sensors_data.shape == 9:
            break
    
    return sensors_data