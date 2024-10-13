from pymavlink import mavutil
import time

class Pixhawk:
    def __init__(self):
        # serial connetion ('/dev/serial0', baud = 57600)
        # intranet connetion ('127.0.0.1:12550)
        self.connection = mavutil.mavlink_connection('COM7', baud = 115200)
        self.acc_current = [0, 0, 0] # current acceleration [x, y, z]
        self.acc_old = [0, 0, 0] # old accelerantion [x, y, z]
        self.gyro = [0, 0, 0] # [x, y, z]
        self.mag = [0, 0, 0] # [x, y, z]
        self.vel = [0, 0, 0] # [x, y, z]

        self.current_time = time.time()
        self.old_time = self.current_time

        self.send_heartbeat()

    def send_heartbeat(self):
        self.connection.mav.heartbeat_send(mavutil.mavlink.MAV_TYPE_GCS,
                                            mavutil.mavlink.MAV_AUTOPILOT_INVALID, 0, 0, 0)
    
    def update_data(self):
        msg1 = False
        msg2 = False

        while not msg1 or not msg2:
            msg = self.connection.recv_match()

            if msg is None:
                continue

            if msg.get_type() == 'HIGHRES_IMU':
                self.acc_old = self.acc_current[:]
                self.acc_current[0] = msg.xacc
                self.acc_current[1] = msg.yacc
                self.acc_current[2] = msg.zacc

                self.old_time = self.current_time
                self.current_time = time.time()

                self.gyro[0] = msg.xgyro
                self.gyro[1] = msg.ygyro
                self.gyro[2] = msg.zgyro
                self.mag[0] = msg.xmag
                self.mag[1] = msg.ymag
                self.mag[2] = msg.zmag

                msg1 = True
            if msg.get_type() == 'LOCAL_POSITION_NED':
                self.vel[0] = msg.vx 
                self.vel[1] = msg.vy
                self.vel[2] = msg.vz

                msg2 = True
    def get_acc(self):
        return self.acc_current
    
    def get_gyro(self):
        return self.gyro
    
    def get_mag(self):
        return self.mag
    
    def get_vel(self):
        return self.vel