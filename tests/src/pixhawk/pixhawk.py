from pymavlink import mavutil
import time

class Pixhawk:
    def __init__(self):
        # serial connetion ('/dev/serial0', baud = 57600)
        # intranet connetion ('127.0.0.1:12550)
        self.connection = mavutil.mavlink_connection('/dev/ttyACM0', baud = 115200)
        self.acc_current = [0, 0, 0] # current acceleration [x, y, z]
        self.acc_old = [0, 0, 0] # old accelerantion [x, y, z]
        self.gyro = [0, 0, 0] # [x, y, z]
        self.mag = [0, 0, 0] # [x, y, z]
        self.vel = [0, 0, 0] # [x, y, z]
        self.current_time = time.time()
        self.old_time = self.current_time
    
    def update_data(self):
        msg = self.connection.recv_match(type='HIGHRES_IMU', blocking=True)
        
        self.acc_old = self.acc_current[:]
        self.acc_current[0] = msg.xacc
        self.acc_current[1] = msg.yacc
        self.acc_current[2] = msg.zacc # -g
        self.old_time = self.current_time
        self.current_time = time.time()
        self.velocity_update()
        self.gyro[0] = msg.xgyro
        self.gyro[1] = msg.ygyro
        self.gyro[2] = msg.zgyro
        self.mag[0] = msg.xmag
        self.mag[1] = msg.ymag
        self.mag[2] = msg.zmag

    def velocity_update(self):
        delta_time = self.current_time - self.old_time

        self.vel[0] += delta_time * (self.acc_current[0] + self.acc_old[0]) / 2
        self.vel[1] += delta_time * (self.acc_current[1] + self.acc_old[1]) / 2
        self.vel[2] += delta_time * (self.acc_current[2] + self.acc_old[2]) / 2

    def get_acc(self):
        return self.acc_current
    
    def get_gyro(self):
        return self.gyro
    
    def get_mag(self):
        return self.mag
    
    def get_vel(self):
        return self.vel