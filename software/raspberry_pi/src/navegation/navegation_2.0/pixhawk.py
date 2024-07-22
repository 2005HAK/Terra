from pymavlink import mavutil

class Pixhawk:
    def __init__(self):
        self.connection = mavutil.mavlink_connection('127.0.0.1:14550')
        self.xacc = 0
        self.yacc = 0
        self.zacc = 0
        self.xgyro = 0
        self.ygyro = 0
        self.zgyro = 0
        self.xmag = 0
        self.ymag = 0
        self.zmag = 0
        self.update_data()
    
    def update_data(self):
        is_simstate_valid = False
        is_scaledimu_valid = False

        while not is_scaledimu_valid or not is_simstate_valid:
            msg = self.connection.recv_match()

            if not msg:
                continue
            if msg.get_type() == 'SIMSTATE':
                self.xacc = msg.xacc
                self.yacc = msg.yacc
                self.zacc = msg.zacc
                self.xgyro = msg.xgyro
                self.ygyro = msg.ygyro
                self.zgyro = msg.zgyro
                is_simstate_valid = True
            if msg.get_type() == 'SCALED_IMU2':
                self.xmag = msg.xmag
                self.ymag = msg.ymag
                self.zmag = msg.zmag
                is_scaledimu_valid = True
                
    def get_connection(self):
        return self.connection
    
    def get_acc(self):
        self.update_data()
        return [self.xacc, self.yacc, self.zacc]
    
    def get_gyro(self):
        self.update_data()
        return [self.xgyro, self.ygyro, self.zgyro]
    
    def get_mag(self):
        self.update_data()
        return [self.xmag, self.ymag, self.zmag]