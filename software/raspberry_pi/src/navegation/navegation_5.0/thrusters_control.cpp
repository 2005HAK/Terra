#include <iostream>
#include <array>
#include <string>
#include <thread>
#include <chrono>
#include <wiringPi.h>
#include "auv_error.cpp"

using namespace std;
using namespace chrono;
using namespace this_thread;

// Defines the power maximum that thrusters can receive (in %)
double POWER_MAX = 25;

//Frequency used for the PWM signal
int FREQUENCY = 200;

/** Thrusters pins
 * 0 - Front left 
 * 1 - Front right
 * 2 - Middle right
 * 3 - Middle left
 * 4 - Back right
 * 5 - Back left 
 */
array<int, 6> PINS = {17, 18, 22, 23, 24, 27};

// PWM form each percentage value
array <int, 201> PWM_VALUES = {1102, 1107, 1111, 1115, 1119, 1122, 1125, 1128, 1131, 1134, 1137, 1139, 1142, 1145, 1147, 1150, 1152, 1155, 1158, 1160, 1163, 1166, 1169, 1172, 1175, 1178, 1181, 1184, 1187, 1190, 1193, 1196, 1199, 1202, 1205, 1207, 1210, 1212, 1215, 1218, 1220, 1223, 1225, 1228, 1231, 1234, 1237, 1240, 1244, 1247, 1251, 1255, 1258, 1262, 1265, 1268, 1272, 1275, 1278, 1281, 1284, 1288, 1291, 1294, 1298, 1302, 1305, 1309, 1313, 1317, 1321, 1325, 1329, 1333, 1336, 1340, 1344, 1348, 1352, 1355, 1359, 1363, 1368, 1372, 1377, 1382, 1386, 1391, 1396, 1401, 1405, 1410, 1415, 1420, 1426, 1432, 1439, 1447, 1455, 1464, 1500, 1540, 1549, 1557, 1564, 1570, 1576, 1582, 1588, 1593, 1598, 1603, 1608, 1612, 1617, 1621, 1626, 1631, 1635, 1640, 1644, 1648, 1653, 1657, 1661, 1665, 1668, 1672, 1676, 1680, 1684, 1688, 1691, 1695, 1699, 1702, 1706, 1710, 1714, 1717, 1721, 1724, 1727, 1731, 1734, 1737, 1740, 1744, 1747, 1751, 1754, 1758, 1761, 1764, 1767, 1770, 1773, 1776, 1779, 1781, 1784, 1786, 1789, 1791, 1794, 1797, 1800, 1803, 1806, 1809, 1812, 1815, 1818, 1820, 1823, 1825, 1828, 1830, 1833, 1835, 1838, 1840, 1843, 1846, 1849, 1852, 1855, 1858, 1861, 1864, 1867, 1870, 1873, 1876, 1879, 1883, 1886, 1890, 1893, 1897, 1900};

/**
 * @brief Actions the AUV can perform
 * 
 * Actions:
 * 
 * - UP         : move the AUV up, turning on the thrusters 3 and 6 in the forward direction.
 * 
 * - DOWN       : move the AUV down, turning on the thrusters 3 and 6 in the reverse direction.
 * 
 * - FORWARD    : move the AUV front, turning on the thrusters 1 and 2 in the reverse direction and the thrusters 4 and 5 in the forward direction.
 * 
 * - BACKWARD   : move the AUV back, turning on the thrusters 1 and 2 in the forward direction and the thrusters 4 and 5 in the reverse direction.
 * 
 * - RIGHT      : move the AUV right, turning on the thrusters 2 and 4 in the forward direction and the thrusters 1 and 5 in the reverse direction.
 * 
 * - LEFT       : move the AUV left, turning on the thrusters 1 and 5 in the forward direction and the thrusters 2 and 4 in the reverse direction.
 * 
 * - TURN RIGHT : turn the AUV right, turning on the thrusters 2 and 5 in the forward direction and the thrusters 1 and 4 in the reverse direction.
 * 
 * - TURN LEFT  : turn the AUV left, turning on the thrusters 1 and 4 in the forward direction and the thrusters 2 and 5 in the reverse direction.
 */
enum class Action{
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT,
    UP,
    DOWN,
    TURNRIGHT,
    TURNLEFT,
    STAY
};

/**
 * @brief Converts the action name to string
 * 
 * @param action Action you want to convert to string
 * 
 * @return Action in string format
 */
string actionToString(Action action){
    switch (action){
        case Action::FORWARD:
            return "FORWARD";
        case Action::BACKWARD:
            return "BACKWARD";
        case Action::RIGHT:
            return "RIGHT";
        case Action::LEFT:
            return "LEFT";
        case Action::UP:
            return "UP";
        case Action::DOWN:
            return "DOWN";
        case Action::TURNRIGHT:
            return "TURNRIGHT";
        case Action::TURNLEFT:
            return "TURNLEFT";
        case Action::STAY:
            return "STAY";
        default:
            return "UNKNOWN";
    }
}

class ThrustersControl{
    private:
        array<Thruster, 6> thrusters;

    public:
        ThrustersControl(){
            cout << "Starting thrusters..." << endl;
            
            if(wiringPiSetup() == -1) throw FailedConnectThrusters();

            initializeThrusters();

            cout << "Engines thrusters" << endl;
        }

        void initializeThrusters(){
            for(int i = 0; i < 6; i++){
                if(PINS[i] == 27 || PINS[i] == 22) Thruster thruster(PINS[i], -14);
                else Thruster thruster(PINS[i], 0);

                thrusters[i] = thruster;
            }

            sleep_for(seconds(7));
        }

        /**
         * @brief Function that takes an argument with movement instruction and decides how the thrusters will be activated
         * 
         * @param action action that the AUV must perform
         * @param value value in percentage that the thrusters must perform
         */
        void defineAction(Action action, int value){
            cout << "Action: " + actionToString(action) << ", Power: " + value << endl;

            switch (action){
                case Action::UP:
                    thrusters[2].move(value);
                    thrusters[5].move(value);
                    break;
                case Action::DOWN:
                    thrusters[2].move(-value);
                    thrusters[5].move(-value);
                    break;
                case Action::FORWARD:
                    thrusters[0].move(-value);
                    thrusters[1].move(-value);
                    thrusters[3].move(value);
                    thrusters[4].move(value);
                    break;
                case Action::BACKWARD:
                    thrusters[0].move(value);
                    thrusters[1].move(value);
                    thrusters[3].move(-value);
                    thrusters[4].move(-value);
                    break;
                case Action::RIGHT:
                    thrusters[0].move(-value);
                    thrusters[1].move(value);
                    thrusters[3].move(value);
                    thrusters[4].move(-value);
                    break;
                case Action::LEFT:
                    thrusters[0].move(value);
                    thrusters[1].move(-value);
                    thrusters[3].move(-value);
                    thrusters[4].move(value);
                    break;
                case Action::TURNRIGHT:
                    thrusters[0].move(-value);
                    thrusters[1].move(value);
                    thrusters[3].move(-value);
                    thrusters[4].move(value);
                    break;
                case Action::TURNLEFT:
                    thrusters[0].move(value);
                    thrusters[1].move(-value);
                    thrusters[3].move(value);
                    thrusters[4].move(-value);
                    break;
                case Action::STAY:
                    thrusters[0].move(0);
                    thrusters[1].move(0);
                    thrusters[3].move(0);
                    thrusters[4].move(0);
                    break;
            }
        }

        /**
         * @brief Finish the thrusters
         */
        void finish(){
            cout << "Turning off the thrusters..." << endl;

            //finalizar

            cout << "Thrusters off" << endl;

        }
};

class Thruster{
    private:
        int pin, stablePower, currentPower;

    public:
        /**
         * @brief Initializes the object thruster
         * 
         * @param pin pin connected in the thrusters
         * @param stablePower power for this thurster to stabilize the AUV (Vx: 0, Vy: 0, Vz: 0)
         */
        Thruster(int pin, int stablePower) : pin(pin), stablePower(stablePower){
            currentPower = 0;
        }

        void initThruster(){

        }

        void move(double value){

        }

        int convertValue(double value){

        }

        void finishesThruster(){

        }
};