from pymavlink import mavutil
import time
import math as m
from AUVError import CollisionDetected

class Pixhawk:
    def __init__(self):
        # serial connetion ('/dev/serial0', baud = 57600)
        # intranet connetion ('127.0.0.1:12550)
        self.connection = mavutil.mavlink_connection('/dev/serial0', baud = 57600)
        self.acc_current = [0, 0, 0] # current acceleration [x, y, z]
        self.acc_old = [0, 0, 0] # old accelerantion [x, y, z]
        self.gyro = [0, 0, 0] # [x, y, z]
        self.mag = [0, 0, 0] # [x, y, z]
        self.vel = [0, 0, 0] # [x, y, z]
        self.current_time = time.time()
        self.old_time = self.current_time

        self.send_heartbeat()

    def send_heartbeat(self):
        """
        Sends the heartbeat to pixhawk for establish connection
        """
        self.connection.mav.heartbeat_send(mavutil.mavlink.MAV_TYPE_GCS,
                                            mavutil.mavlink.MAV_AUTOPILOT_INVALID, 0, 0, 0)
    
    def update_data(self):
        is_simstate_valid = False
        is_scaledimu_valid = False

        while not is_scaledimu_valid or not is_simstate_valid:
            msg = self.connection.recv_match()
            # Avaliar possibilidade de ja filtrar SIMSTATE e SCALED_IMU2 nesse ponto do código ao invés de usar os ifs
            # falar com o Moro sobre isso
            if not msg:
                continue
            if msg.get_type() == 'SIMSTATE':
                self.acc_old = self.acc_current[:]
                self.acc_current[0] = msg.xacc
                self.acc_current[1] = msg.yacc
                self.acc_current[2] = msg.zacc # -9.81
                self.old_time = self.current_time
                self.current_time = time.time()
                self.velocity_update()
                self.gyro[0] = msg.xgyro
                self.gyro[1] = msg.ygyro
                self.gyro[2] = msg.zgyro
                is_simstate_valid = True
            if msg.get_type() == 'SCALED_IMU2':
                self.mag[0] = msg.xmag
                self.mag[1] = msg.ymag
                self.mag[2] = msg.zmag
                is_scaledimu_valid = True
    
    def velocity_update(self):
        delta_time = self.current_time - self.old_time

        self.vel[0] += delta_time * (self.acc_current[0] + self.acc_old[0]) / 2
        self.vel[1] += delta_time * (self.acc_current[1] + self.acc_old[1]) / 2
        self.vel[2] += delta_time * (self.acc_current[2] + self.acc_old[2]) / 2

    def collision_detect(self):
        """
        Detects whether the AUV has crashed based on acceleration data from Pixhawk

        :param connection: Connection with Pixhawk

        :return: If the AUV collided
        """

        # Acceleration limit to consider a collision
        ACC_LIMIT = 15
        acceleration = self.get_acc()

        if any(m.fabs(acc) > ACC_LIMIT for acc in acceleration):
            raise CollisionDetected(acceleration, self.current_time)
    
    def get_acc(self):
        return self.acc_current
    
    def get_gyro(self):
        return self.gyro
    
    def get_mag(self):
        return self.mag
    
    def get_vel(self):
        return self.vel