from pymavlink import mavutil
import time

class Pixhawk:
    def __init__(self):
        # serial connetion ('/dev/serial0', baud = 57600)
        # intranet connetion ('127.0.0.1:12550)
        self.connection = mavutil.mavlink_connection('COM5', baud = 115200)
        self.acc = [0, 0, 0] # current acceleration [x, y, z]
        self.gyro = [0, 0, 0] # [x, y, z]
        self.mag = [0, 0, 0] # [x, y, z]
        self.vel = [0, 0, 0] # [x, y, z]

        self.current_time = time.time()
        self.old_time = self.current_time

        self.send_heartbeat()

    def send_heartbeat(self):
        self.connection.mav.heartbeat_send(mavutil.mavlink.MAV_TYPE_SUBMARINE,
                                            mavutil.mavlink.MAV_AUTOPILOT_INVALID, 0, 0, 0)
    
    def update_data(self):
        is_highres_imu_valid = False
        is_local_position_ned_valid = False

        self.old_time = self.current_time

        while not is_local_position_ned_valid or not is_highres_imu_valid:
            msg = self.connection.recv_match(type=['GLOBAL_POSITION_INT', 'SCALED_IMU2'])

            if msg is None:
                continue

            if msg.get_type() == 'SCALED_IMU2':
                # conferir as conversões
                convert_to_ms = 9.80665 / 1000
                convert_to_rad = 1 / 1000
                convert_to_t = 1 / 10000000

                self.acc[0] = msg.xacc * convert_to_ms
                self.acc[1] = msg.yacc * convert_to_ms
                self.acc[2] = msg.zacc * convert_to_ms

                self.gyro[0] = msg.xgyro * convert_to_rad
                self.gyro[1] = msg.ygyro * convert_to_rad
                self.gyro[2] = msg.zgyro * convert_to_rad
                # Dados de mag não funcionam
                self.mag[0] = msg.xmag
                self.mag[1] = msg.ymag
                self.mag[2] = msg.zmag
                
                is_highres_imu_valid = True
            if msg.get_type() == 'GLOBAL_POSITION_INT':
                self.vel[0] = msg.vx / 100
                self.vel[1] = msg.vy / 100
                self.vel[2] = msg.vz / 100

                is_local_position_ned_valid = True

        self.current_time = time.time()

    def get_acc(self):
        return self.acc
    
    def get_gyro(self):
        return self.gyro
    
    def get_mag(self):
        return self.mag
    
    def get_vel(self):
        return self.vel