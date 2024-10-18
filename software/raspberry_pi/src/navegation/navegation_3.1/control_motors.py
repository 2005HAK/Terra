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

PWM_VALUES = [1102, 1107, 1111, 1115, 1119, 1122, 1125, 1128, 1131, 1134, 1137, 1139, 1142, 1145, 1147, 1150, 1152, 1155, 1158, 1160, 1163, 1166, 1169, 1172, 1175, 1178, 1181, 1184, 1187, 1190, 1193, 1196, 1199, 1202, 1205, 1207, 1210, 1212, 1215, 1218, 1220, 1223, 1225, 1228, 1231, 1234, 1237, 1240, 1244, 1247, 1251, 1255, 1258, 1262, 1265, 1268, 1272, 1275, 1278, 1281, 1284, 1288, 1291, 1294, 1298, 1302, 1305, 1309, 1313, 1317, 1321, 1325, 1329, 1333, 1336, 1340, 1344, 1348, 1352, 1355, 1359, 1363, 1368, 1372, 1377, 1382, 1386, 1391, 1396, 1401, 1405, 1410, 1415, 1420, 1426, 1432, 1439, 1447, 1455, 1464, 1500, 1540, 1549, 1557, 1564, 1570, 1576, 1582, 1588, 1593, 1598, 1603, 1608, 1612, 1617, 1621, 1626, 1631, 1635, 1640, 1644, 1648, 1653, 1657, 1661, 1665, 1668, 1672, 1676, 1680, 1684, 1688, 1691, 1695, 1699, 1702, 1706, 1710, 1714, 1717, 1721, 1724, 1727, 1731, 1734, 1737, 1740, 1744, 1747, 1751, 1754, 1758, 1761, 1764, 1767, 1770, 1773, 1776, 1779, 1781, 1784, 1786, 1789, 1791, 1794, 1797, 1800, 1803, 1806, 1809, 1812, 1815, 1818, 1820, 1823, 1825, 1828, 1830, 1833, 1835, 1838, 1840, 1843, 1846, 1849, 1852, 1855, 1858, 1861, 1864, 1867, 1870, 1873, 1876, 1879, 1883, 1886, 1890, 1893, 1897, 1900]

class Motors:
    def __init__(self):
        print("Starting engines...")

        self.gpio = pigpio.pi()

        # while(not pi.connect()):
            # pi = pigpio.pi()
        self.motors = []

        self.inicialize_pins()

        print("Engines started")

    def inicialize_pins(self):
        for pin in PINS:
            motor = Motor(pin, self.gpio, 0 if pin != 2 or pin != 3 else 30)
            self.motors.append(motor)

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

            if action == "UP":
                self.motors[2].move(value)
                self.motors[5].move(value)
            elif action == "DOWN":
                self.motors[2].move(-value)
                self.motors[5].move(-value)

            if action == "FRONT":
                self.motors[0].move(-value)
                self.motors[1].move(-value)
                self.motors[3].move(value)
                self.motors[4].move(value)
            if action == "BACK":
                self.motors[0].move(value)
                self.motors[1].move(value)
                self.motors[3].move(-value)
                self.motors[4].move(-value)
            if action == "RIGHT":
                self.motors[0].move(-value)
                self.motors[1].move(value)
                self.motors[3].move(value)
                self.motors[4].move(-value)
            if action == "LEFT":
                self.motors[0].move(value)
                self.motors[1].move(-value)
                self.motors[3].move(-value)
                self.motors[4].move(value)
            if action == "TURN RIGHT":
                self.motors[0].move(-value)
                self.motors[1].move(value)
                self.motors[3].move(-value)
                self.motors[4].move(value)
            if action == "TURN LEFT":
                self.motors[0].move(value)
                self.motors[1].move(-value)
                self.motors[3].move(value)
                self.motors[4].move(-value)
            if action == "STAY":
                self.motors[0].move(0)
                self.motors[1].move(0)
                self.motors[2].move(0)
                self.motors[3].move(0)
                self.motors[4].move(0)
                self.motors[5].move(0)

    def finish(self):
        """
        Finish the motors
        """

        print("Turning off the motors...")

        for motor in self.motors:
            motor.finishes_motor()

        self.gpio.stop()

        print("Motors off")

class Motor:
    def __init__(self, pin, pi, stable_power):
        """
        Initializes the object motor.

        - pin: pin connected in the motor
        - stable_power: power for this motor to stabilize the AUV (Vx: 0, Vy: 0, Vz: 0)
        - current_power: current power set in this motor
        - direction: whether it is forward or backward
        """

        self.pin = pin
        self.stable_power = stable_power
        self.current_power = 0
        self.direction = None
        self.gpio = pi

        self.init_motor()
    
    def init_motor(self):
        self.gpio.set_mode(self.pin, pigpio.OUTPUT)
        self.gpio.set_PWM_frequency(self.pin, FREQUENCY)
        self.gpio.set_servo_pulsewidth(self.pin, 1500)

    def move(self, value):
        useful_power = ((100 - self.stable_power) if value > 0 else (100 + self.stable_power)) * (value / 100)
        self.current_power = self.stable_power + useful_power
        self.gpio.set_servo_pulsewidth(self.pin, self.convert_value(self.current_power))

    def convert_value(self, value):
        return PWM_VALUES[100 + int(value)] if value < 101 and value > -101 else PWM_VALUES[100 + int(self.stable_power)]
    
    def finishes_motor(self):
        self.gpio.set_servo_pulsewidth(1500)