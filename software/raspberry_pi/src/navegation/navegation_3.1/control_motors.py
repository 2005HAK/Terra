import pigpio
import time

FREQUENCY = 200

# Pins motors !!!!IMPORTANT: SHOULD BE DEFINED!!!!
PINS = [1, 2, 3, 4, 5, 6]
# 0 - Front left 
# 1 - Front right
# 2 - Middle right
# 3 - Middle left
# 4 - Back right
# 5 - Back left 

# value for the motor to stop
REST_VALUE = 1500

PWM_VALUES = [1500, 1464, 1455, 1447, 1439, 1432, 1426, 1420, 1415, 1410, 1405, 1401, 1396, 1391, 1386, 1382, 1377, 1372, 1368, 1363, 1359, 1355, 1352, 1348, 1344, 1340, 1336, 1333, 1329, 1325, 1321, 1317, 1313, 1309, 1305, 1302, 1298, 1294, 1291, 1288, 1284, 1281, 1278, 1275, 1272, 1268, 1265, 1262, 1258, 1255, 1251, 1247, 1244, 1240, 1237, 1234, 1231, 1228, 1225, 1223, 1220, 1218, 1215, 1212, 1210, 1207, 1205, 1202, 1199, 1196, 1193, 1190, 1187, 1184, 1181, 1178, 1175, 1172, 1169, 1166, 1163, 1160, 1158, 1155, 1152, 1150, 1147, 1145, 1142, 1139, 1137, 1134, 1131, 1128, 1125, 1122, 1119, 1115, 1111, 1107, 1102, 1500, 1540, 1549, 1557, 1564, 1570, 1576, 1582, 1588, 1593, 1598, 1603, 1608, 1612, 1617, 1621, 1626, 1631, 1635, 1640, 1644, 1648, 1653, 1657, 1661, 1665, 1668, 1672, 1676, 1680, 1684, 1688, 1691, 1695, 1699, 1702, 1706, 1710, 1714, 1717, 1721, 1724, 1727, 1731, 1734, 1737, 1740, 1744, 1747, 1751, 1754, 1758, 1761, 1764, 1767, 1770, 1773, 1776, 1779, 1781, 1784, 1786, 1789, 1791, 1794, 1797, 1800, 1803, 1806, 1809, 1812, 1815, 1818, 1820, 1823, 1825, 1828, 1830, 1833, 1835, 1838, 1840, 1843, 1846, 1849, 1852, 1855, 1858, 1861, 1864, 1867, 1870, 1873, 1876, 1879, 1883, 1886, 1890, 1893, 1897, 1900]

class Motors:
    def __init__(self):
        print("Starting engines...")

        self.pi = pigpio.pi()

        # while(not pi.connect()):
            # pi = pigpio.pi()
        self.motors = []

        self.inicialize_pins()

        print("Engines started")

    def inicialize_pins(self):
        for pin in PINS:
            motor = Motor()

        time.sleep(7)

    def define_action(self, actions):
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
            print(f"Action: {action}, Power: {value}")

            forward_value = convert_forward(value)
            reverse_value = convert_reverse(value)

            if action == "UP":
                self.pi.set_servo_pulsewidth(PINS[2], forward_value)
                self.pi.set_servo_pulsewidth(PINS[5], forward_value)
            elif action == "DOWN":
                self.pi.set_servo_pulsewidth(PINS[2], reverse_value)
                self.pi.set_servo_pulsewidth(PINS[5], reverse_value)

            if action == "FRONT":
                self.pi.set_servo_pulsewidth(PINS[0], reverse_value)
                self.pi.set_servo_pulsewidth(PINS[1], reverse_value)
                self.pi.set_servo_pulsewidth(PINS[3], forward_value)
                self.pi.set_servo_pulsewidth(PINS[4], forward_value)
            if action == "BACK":
                self.pi.set_servo_pulsewidth(PINS[0], forward_value)
                self.pi.set_servo_pulsewidth(PINS[1], forward_value)
                self.pi.set_servo_pulsewidth(PINS[3], reverse_value)
                self.pi.set_servo_pulsewidth(PINS[4], reverse_value)
            if action == "RIGHT":
                self.pi.set_servo_pulsewidth(PINS[0], reverse_value)
                self.pi.set_servo_pulsewidth(PINS[1], forward_value)
                self.pi.set_servo_pulsewidth(PINS[3], forward_value)
                self.pi.set_servo_pulsewidth(PINS[4], reverse_value)
            if action == "LEFT":
                self.pi.set_servo_pulsewidth(PINS[0], forward_value)
                self.pi.set_servo_pulsewidth(PINS[1], reverse_value)
                self.pi.set_servo_pulsewidth(PINS[3], reverse_value)
                self.pi.set_servo_pulsewidth(PINS[4], forward_value)
            if action == "TURN RIGHT":
                self.pi.set_servo_pulsewidth(PINS[0], reverse_value)
                self.pi.set_servo_pulsewidth(PINS[1], forward_value)
                self.pi.set_servo_pulsewidth(PINS[3], reverse_value)
                self.pi.set_servo_pulsewidth(PINS[4], forward_value)
            if action == "TURN LEFT":
                self.pi.set_servo_pulsewidth(PINS[0], forward_value)
                self.pi.set_servo_pulsewidth(PINS[1], reverse_value)
                self.pi.set_servo_pulsewidth(PINS[3], forward_value)
                self.pi.set_servo_pulsewidth(PINS[4], reverse_value)
            if action == "STOP":
                self.pi.set_servo_pulsewidth(PINS[0], REST_VALUE)
                self.pi.set_servo_pulsewidth(PINS[1], REST_VALUE)
                self.pi.set_servo_pulsewidth(PINS[2], REST_VALUE)
                self.pi.set_servo_pulsewidth(PINS[3], REST_VALUE)
                self.pi.set_servo_pulsewidth(PINS[4], REST_VALUE)
                self.pi.set_servo_pulsewidth(PINS[5], REST_VALUE)

    def finish(self):
        """
        Finish the motors
        """
        print("Turning off the engines...")

        for pin in PINS:
            self.pi.set_PWM_dutycycle(pin, 0)
        self.pi.stop()

        print("Engines off")

def convert_forward(value):
    return PWM_VALUES [101 + int(value)]

def convert_reverse(value):
    return PWM_VALUES [int(value)]

class Motor:
    def __init__(self, pin, pi):
        """
        Initializes the object motor.

        - pin: pin connected in the motor
        - stable_power: power for this motor to stabilize the AUV (Vx: 0, Vy: 0, Vz: 0)
        - current_power: current power set in this motor
        - direction: whether it is forward or backward
        """

        self.pin = pin
        self.stable_power = 0
        self.current_power = 0
        self.direction = None
        self.pi = pi
    
    def init_motor(self):
        self.pi.set_mode(self.pin, pigpio.OUTPUT)
        self.pi.set_PWM_frequency(self.pin, FREQUENCY)
        self.pi.set_servo_pulsewidth(self.pin, REST_VALUE)

