from pymavlink import mavutil
from time import time
import math as m
from AUVError import CollisionDetected, PixhawkHighTempError, RaspberryHighTempError

# conferir as conversões
CONV_TO_MS = 9.80665 / 1000  # convert mG to m/s²
CONV_TO_RAD = 1 / 1000   # convert mrad/s to rad/s
CONV_TO_T = 1 / 10000000 # convert mgauss to Tesla

# Acceleration limit to consider a collision
ACC_LIMIT = 15

# Critical temperatures
MAX_TEMP_PIXHAWK = 50
MAX_TEMP_RASPBERRY = 85
MAX_TEMP_JETSON = 60

class Sensors:
    def __init__(self):
        """
        Initializes the Sensors class, establishing a connection to Pixhawk and setting initial values 
        for acceleration, gyro, magnetometer, velocity, and temperature sensors.
        """

        # serial connetion ('/dev/serial0', baud = 57600)
        # intranet connetion ('127.0.0.1:12550)
        self.connection = mavutil.mavlink_connection('/dev/serial0', baud = 57600)
        self.acc = [0, 0, 0] # current acceleration [x, y, z]
        self.gyro = [0, 0, 0] # [x, y, z]
        self.mag = [0, 0, 0] # [x, y, z]
        self.vel = [0, 0, 0] # [x, y, z]
        self.temp_pixhawk = None
        self.temp_raspberry = None

        self.current_time = time()
        self.old_time = self.current_time

        self.__send_heartbeat()

    def update_data(self):
        """
        Continuously updates sensor data, fetching new information from Pixhawk and temperature sensors
        """

        while True:
            self.old_time = self.current_time

            self.__update_data_imu()
            self.__update_data_temp()

            self.current_time = time()

    def __send_heartbeat(self):
        """
        Sends the heartbeat to pixhawk for establish connection
        """
        self.connection.mav.heartbeat_send(mavutil.mavlink.MAV_TYPE_GCS, mavutil.mavlink.MAV_AUTOPILOT_INVALID, 0, 0, 0)
    
    def __update_data_imu(self):
        """
        updates acceleration, gyroscope, magnetometer and speed data
        """

        is_scaled_imu2_valid = 0
        is_global_position_int_valid = 0

        while not is_global_position_int_valid or not is_scaled_imu2_valid:
                msg = self.connection.recv_match(type=['GLOBAL_POSITION_INT', 'SCALED_IMU2'])

                if msg is None:
                    continue

                if msg.get_type() == 'SCALED_IMU2':
                    self.acc = [msg.xacc * CONV_TO_MS, msg.yacc * CONV_TO_MS, msg.zacc * CONV_TO_MS]

                    self.gyro = [msg.xgyro * CONV_TO_RAD, msg.ygyro * CONV_TO_RAD, msg.zgyro * CONV_TO_RAD]
                    # Dados de mag não funcionam
                    self.mag = [msg.xmag, msg.ymag, msg.zmag]
                    
                    is_scaled_imu2_valid = 1
                if msg.get_type() == 'GLOBAL_POSITION_INT':
                    self.vel = [msg.vx / 100, msg.vy / 100, msg.vz / 100]

                    is_global_position_int_valid = 1

    # IMPLEMENTAR
    def __update_data_temp(self):
        """
        Updates system temperature data
        """

        pass

    def collision_detect(self):
        """
        Detects whether the AUV has crashed based on acceleration data from Pixhawk
        """

        acceleration = self.get_acc()

        if any(m.fabs(acc) > ACC_LIMIT for acc in acceleration):
            raise CollisionDetected(acceleration, self.current_time)
    
    # TERMINAR
    def detect_overheat(self):
        """
        checks whether the systems temperature is safe
        """

        if self.temp_pixhawk > MAX_TEMP_PIXHAWK:
            raise PixhawkHighTempError(self.temp_pixhawk, self.current_time)
        if self.temp_raspberry > MAX_TEMP_RASPBERRY:
            raise RaspberryHighTempError(self.temp_raspberry, self.current_time)
    
    def get_acc(self):
        return self.acc
    
    def get_gyro(self):
        return self.gyro
    
    def get_mag(self):
        return self.mag
    
    def get_vel(self):
        return self.vel