#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <wiringPi.h>
#include "auv_error.cpp"

using namespace std;
using namespace chrono;
using namespace this_thread;

// Defines the power maximum that thrusters can receive (in %)
const double POWER_MAX = 25;

//Frequency used for the PWM signal
const double FREQUENCY = 200;

/** Thrusters pins
 * 0 - Front left 
 * 1 - Front right
 * 2 - Middle right
 * 3 - Middle left
 * 4 - Back right
 * 5 - Back left 
 */
const array<int, 6> PINS = {17, 18, 22, 23, 24, 27};

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
        case Action::FORWARD: return "FORWARD";
        case Action::BACKWARD: return "BACKWARD";
        case Action::RIGHT: return "RIGHT";
        case Action::LEFT: return "LEFT";
        case Action::UP: return "UP";
        case Action::DOWN: return "DOWN";
        case Action::TURNRIGHT: return "TURNRIGHT";
        case Action::TURNLEFT: return "TURNLEFT";
        case Action::STAY: return "STAY";
        default: return "UNKNOWN";
    }
}

class Thruster{
    private:
        int pin, stablePower, currentPower;
        int pwmRange = 1000;

    public:
        /**
         * @brief Initializes the object thruster
         * 
         * @param pin pin connected in the thrusters
         * @param stablePower power for this thurster to stabilize the AUV (Vx: 0, Vy: 0, Vz: 0)
         */
        Thruster(int pin, int stablePower) : pin(pin), stablePower(stablePower), currentPower(0){
            initThruster();
        }

        /**
         * @brief Initializes the thruster
         * 
         * Defines the the pin as output, defines the PWM mode as MS (mark:space), defines the clock divisor
         *  (default clock 19.2MHz), defines the PWM value range and writes 0 in pin, which corresponds 1500us
         */
        void initThruster(){
            pinMode(this->pin, PWM_OUTPUT);
            pwmSetMode(PWM_MODE_MS); // Entender e alterar se necessario
            pwmSetClock(19200000 / (FREQUENCY * this->pwmRange));
            pwmSetRange(pwmRange); // 
            pwmWrite(this->pin, convertValue(0));
        }

        /**
         * @brief Sets a PWM value into dutycycle value from a percentage value
         * 
         * @param value A percentage value from -100 to 100 
         */
        void move(double value){
            int pwmValue = percentageToDutycycle(value);
            pwmWrite(pin, pwmValue);
        }

        /**
         * @brief Converts a percentage value in dutycycle value
         * 
         * @param value A percentage value from -100 to 100
         * 
         * @return Corresponding dutycycle value
         */
        int percentageToDutycycle(double value){
            // ENTENDER POR QUE ISSO SÓ FUNCIONOU COM DOUBLE !!!!!!!!!!!!!!
            int minPWMus = 1100, maxPWMus = 1900;

            double usefulPower = (value > 0 ? (100 - stablePower) : (100 + stablePower)) * (value / 100);
            currentPower = max(-POWER_MAX, min(stablePower + usefulPower, POWER_MAX));

            return (FREQUENCY / 1000000) * (minPWMus + ((maxPWMus - minPWMus) / 2) * (1 + (currentPower / 100))) * pwmRange;
        }

        /**
         * @brief Turns off the thruster
         * 
         * Defines the dutycycle to 0
         */
        void finishesThruster(){
            pwmWrite(this->pin, 0);
        }
};

class ThrustersControl{
    private:
        vector<Thruster> thrusters;

    public:
        /**
         * @brief Initializes the thrusters control
         * 
         * Active the wiringPi service and initializes each thruster
         */
        ThrustersControl(){
            cout << "Starting thrusters..." << endl;
            
            if(wiringPiSetup() == -1) throw FailedConnectThrusters();

            initializeThrusters();

            cout << "Engines thrusters" << endl;
        }

        /**
         * @brief Initializes each AUV thruster
         * 
         * Creates each AUV thruster, adds each one to the thrusters vector and waits 7 seconds for the
         *  thrusters to start
         */
        void initializeThrusters(){
            for(int i = 0; i < PINS.size(); i++){
                if(PINS[i] == 27 || PINS[i] == 22) thrusters.emplace_back(PINS[i], -14);
                else thrusters.emplace_back(PINS[i], 0);
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

            for(int i = 0; i < thrusters.size(); i++) thrusters[i].finishesThruster();

            cout << "Thrusters off" << endl;
        }
};