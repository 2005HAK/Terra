#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <wiringPi.h>
#include "auv_error.h"

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

struct Decision{
    Action action = Action::NONE;
    double value = -1;
};

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
            pwmWrite(this->pin, percentageToDutycycle(0));
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
         * @param decision struct with the action decision that will be executed by the AUV and with what power
         */
        void defineAction(Decision decision){
            cout << "Action: " << actionToString(decision.action) << ", Power: " << decision.value << endl;

            switch (decision.action){
                case Action::UP:
                    thrusters[2].move(decision.value);
                    thrusters[5].move(decision.value);
                    break;
                case Action::DOWN:
                    thrusters[2].move(-decision.value);
                    thrusters[5].move(-decision.value);
                    break;
                case Action::FORWARD:
                    thrusters[0].move(-decision.value);
                    thrusters[1].move(-decision.value);
                    thrusters[3].move(decision.value);
                    thrusters[4].move(decision.value);
                    break;
                case Action::BACKWARD:
                    thrusters[0].move(decision.value);
                    thrusters[1].move(decision.value);
                    thrusters[3].move(-decision.value);
                    thrusters[4].move(-decision.value);
                    break;
                case Action::RIGHT:
                    thrusters[0].move(-decision.value);
                    thrusters[1].move(decision.value);
                    thrusters[3].move(decision.value);
                    thrusters[4].move(-decision.value);
                    break;
                case Action::LEFT:
                    thrusters[0].move(decision.value);
                    thrusters[1].move(-decision.value);
                    thrusters[3].move(-decision.value);
                    thrusters[4].move(decision.value);
                    break;
                case Action::TURNRIGHT:
                    thrusters[0].move(-decision.value);
                    thrusters[1].move(decision.value);
                    thrusters[3].move(-decision.value);
                    thrusters[4].move(decision.value);
                    break;
                case Action::TURNLEFT:
                    thrusters[0].move(decision.value);
                    thrusters[1].move(-decision.value);
                    thrusters[3].move(decision.value);
                    thrusters[4].move(-decision.value);
                    break;
                default:
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
