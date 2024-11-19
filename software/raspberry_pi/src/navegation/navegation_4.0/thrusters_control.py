from pigpio import pi, OUTPUT
from time import sleep
from enum import Enum, auto

# Defines the power maximum that thrusters can receive (in %)
POWER_MAX = 25

FREQUENCY = 200

# Pins thrusters !!!!IMPORTANT: SHOULD BE DEFINED!!!!
PINS = [1, 2, 22, 27, 5, 6]
# 0 - Front left 
# 1 - Front right
# 2 - Middle right
# 3 - Middle left
# 4 - Back right
# 5 - Back left 

PWM_VALUES = [1102, 1107, 1111, 1115, 1119, 1122, 1125, 1128, 1131, 1134, 1137, 1139, 1142, 1145, 1147, 1150, 1152, 1155, 1158, 1160, 1163, 1166, 1169, 1172, 1175, 1178, 1181, 1184, 1187, 1190, 1193, 1196, 1199, 1202, 1205, 1207, 1210, 1212, 1215, 1218, 1220, 1223, 1225, 1228, 1231, 1234, 1237, 1240, 1244, 1247, 1251, 1255, 1258, 1262, 1265, 1268, 1272, 1275, 1278, 1281, 1284, 1288, 1291, 1294, 1298, 1302, 1305, 1309, 1313, 1317, 1321, 1325, 1329, 1333, 1336, 1340, 1344, 1348, 1352, 1355, 1359, 1363, 1368, 1372, 1377, 1382, 1386, 1391, 1396, 1401, 1405, 1410, 1415, 1420, 1426, 1432, 1439, 1447, 1455, 1464, 1500, 1540, 1549, 1557, 1564, 1570, 1576, 1582, 1588, 1593, 1598, 1603, 1608, 1612, 1617, 1621, 1626, 1631, 1635, 1640, 1644, 1648, 1653, 1657, 1661, 1665, 1668, 1672, 1676, 1680, 1684, 1688, 1691, 1695, 1699, 1702, 1706, 1710, 1714, 1717, 1721, 1724, 1727, 1731, 1734, 1737, 1740, 1744, 1747, 1751, 1754, 1758, 1761, 1764, 1767, 1770, 1773, 1776, 1779, 1781, 1784, 1786, 1789, 1791, 1794, 1797, 1800, 1803, 1806, 1809, 1812, 1815, 1818, 1820, 1823, 1825, 1828, 1830, 1833, 1835, 1838, 1840, 1843, 1846, 1849, 1852, 1855, 1858, 1861, 1864, 1867, 1870, 1873, 1876, 1879, 1883, 1886, 1890, 1893, 1897, 1900]

class Actions(Enum):
    """
    Actions the AUV can perform

    - UP         : move the AUV up, turning on the thrusters 3 and 6 in the forward direction.
    - DOWN       : move the AUV down, turning on the thrusters 3 and 6 in the reverse direction.
    - FORWARD    : move the AUV front, turning on the thrusters 1 and 2 in the reverse direction and the thrusters 4 and 5 in the forward direction.
    - BACKWARD   : move the AUV back, turning on the thrusters 1 and 2 in the forward direction and the thrusters 4 and 5 in the reverse direction.
    - RIGHT      : move the AUV right, turning on the thrusters 2 and 4 in the forward direction and the thrusters 1 and 5 in the reverse direction.
    - LEFT       : move the AUV left, turning on the thrusters 1 and 5 in the forward direction and the thrusters 2 and 4 in the reverse direction.
    - TURN RIGHT : turn the AUV right, turning on the thrusters 2 and 5 in the forward direction and the thrusters 1 and 4 in the reverse direction.
    - TURN LEFT  : turn the AUV left, turning on the thrusters 1 and 4 in the forward direction and the thrusters 2 and 5 in the reverse direction.
    """

    FORWARD = auto()
    BACKWARD = auto()
    RIGHT = auto()
    LEFT = auto()
    UP = auto()
    DOWN = auto()
    TURNRIGHT = auto()
    TURNLEFT = auto()
    STAY = auto()

class ThrustersControl:
    def __init__(self):
        print("Starting thrusters...")

        self.gpio = pi()

        # while(not pi.connect()):
            # pi = pigpio.pi()

        self.thrusters = []

        self.initialize_pins()

        print("Engines thrusters")

    def initialize_pins(self):
        for pin in PINS:
            if pin == 27 or pin == 22:
                thruster = Thruster(pin, self.gpio, -14)
            else:
                thruster = Thruster(pin, self.gpio, 0)
            self.thrusters.append(thruster)

        sleep(7)

    def define_action(self, actions):
        """
        Function that takes an argument with movement instruction and decides how the thrusters will be activated

        :param actions: actions that the AUV must perform
        :type actions: Actions

        Return: None
        """

        for action, value in actions.items():
            print(f"Action: {action}, Power: {value}")

            if action == Actions.UP:
                self.thrusters[2].move(value)
                self.thrusters[5].move(value)
            elif action == Actions.DOWN:
                self.thrusters[2].move(-value)
                self.thrusters[5].move(-value)

            if action == Actions.FORWARD:
                self.thrusters[0].move(-value)
                self.thrusters[1].move(-value)
                self.thrusters[3].move(value)
                self.thrusters[4].move(value)
            if action == Actions.BACKWARD:
                self.thrusters[0].move(value)
                self.thrusters[1].move(value)
                self.thrusters[3].move(-value)
                self.thrusters[4].move(-value)
            if action == Actions.RIGHT:
                self.thrusters[0].move(-value)
                self.thrusters[1].move(value)
                self.thrusters[3].move(value)
                self.thrusters[4].move(-value)
            if action == Actions.LEFT:
                self.thrusters[0].move(value)
                self.thrusters[1].move(-value)
                self.thrusters[3].move(-value)
                self.thrusters[4].move(value)
            if action == Actions.TURNRIGHT:
                self.thrusters[0].move(-value)
                self.thrusters[1].move(value)
                self.thrusters[3].move(-value)
                self.thrusters[4].move(value)
            if action == Actions.TURNLEFT:
                self.thrusters[0].move(value)
                self.thrusters[1].move(-value)
                self.thrusters[3].move(value)
                self.thrusters[4].move(-value)
            if action == Actions.STAY:
                self.thrusters[0].move(0)
                self.thrusters[1].move(0)
                self.thrusters[2].move(0)
                self.thrusters[3].move(0)
                self.thrusters[4].move(0)
                self.thrusters[5].move(0)

    def finish(self):
        """
        Finish the thrusters
        """

        print("Turning off the thrusters...")

        for thruster in self.thrusters:
            thruster.finishes_thruster()

        self.gpio.stop()

        print("Thrusters off")

class Thruster:
    def __init__(self, pin, pi, stable_power):
        """
        Initializes the object thruster.

        :param pin: pin connected in the thruster
        :type pin: int
        :param stable_power: power for this thruster to stabilize the AUV (Vx: 0, Vy: 0, Vz: 0)
        :type stable_power: int
        :param current_power: current power set in this thruster
        :type current_power: int
        """

        self.pin = pin
        self.stable_power = stable_power
        self.current_power = 0
        self.gpio = pi

        self.init_thruster()
    
    def init_thruster(self):
        self.gpio.set_mode(self.pin, OUTPUT)
        self.gpio.set_PWM_frequency(self.pin, FREQUENCY)
        self.gpio.set_servo_pulsewidth(self.pin, 1500)

    def move(self, value):
        useful_power = ((100 - self.stable_power) if value > 0 else (100 + self.stable_power)) * (value / 100)
        self.current_power = max(min(self.stable_power + useful_power, POWER_MAX), 0)
        self.gpio.set_servo_pulsewidth(self.pin, self.convert_value(self.current_power))

    def convert_value(self, value):
        return PWM_VALUES[100 + int(value)] if value < 101 and value > -101 else PWM_VALUES[100 + int(self.stable_power)]
    
    def finishes_thruster(self):
        self.gpio.set_servo_pulsewidth(1500)