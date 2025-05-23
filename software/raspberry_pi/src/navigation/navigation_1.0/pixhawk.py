from pymavlink import mavutil

master = mavutil.mavlink_connection('127.0.0.1:14550')

def get_data():
    """
    Collects acceleration values on the x, y and z axes, gyroscope values on the x, y and z axes, and magnetometer values on the x, y and z axes

    :param: none

    :return: A list with [xacc, yacc, zacc, xgyro, ygyro, zgyro, xmag, ymag, zmag], where acc is in [m/s²], gyro is in [rad/s] and mag is in [mgauss]
    """
    
    sensors_data = []
    validoSIMSTATE = False
    validoSCALEDIMU2 = False

    while not validoSCALEDIMU2 or not validoSIMSTATE:
        msg = master.recv_match()

        if not msg:
            continue
        if msg.get_type() == 'SIMSTATE' and not validoSIMSTATE:
            sensors_data.append(msg.xacc)
            sensors_data.append(msg.yacc)
            sensors_data.append(msg.zacc)
            sensors_data.append(msg.xgyro)
            sensors_data.append(msg.ygyro)
            sensors_data.append(msg.zgyro)
            validoSIMSTATE = True
        if msg.get_type() == 'SCALED_IMU2' and validoSIMSTATE == True:
            sensors_data.append(msg.xmag)
            sensors_data.append(msg.ymag)
            sensors_data.append(msg.zmag)
            validoSCALEDIMU2 = True
    
    return sensors_data