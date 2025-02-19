#ifndef THRUSTERSCTRL_H
#define THRUSTERSCTRL_H

#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <chrono>
#include <wiringPi.h>
#include "auverror.h"

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
        Thruster(int pin, int stablePower);

        /**
         * @brief Initializes the thruster
         * 
         * Defines the the pin as output, defines the PWM mode as MS (mark:space), defines the clock divisor
         *  (default clock 19.2MHz), defines the PWM value range and writes 0 in pin, which corresponds 1500us
         */
        void initThruster();

        /**
         * @brief Sets a PWM value into dutycycle value from a percentage value
         * 
         * @param value A percentage value from -100 to 100 
         */
        void move(double value);

        /**
         * @brief Converts a percentage value in dutycycle value
         * 
         * @param value A percentage value from -100 to 100
         * 
         * @return Corresponding dutycycle value
         */
        int percentageToDutycycle(double value);

        /**
         * @brief Turns off the thruster
         * 
         * Defines the dutycycle to 0
         */
        void finishesThruster();
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
        ThrustersControl();

        /**
         * @brief Initializes each AUV thruster
         * 
         * Creates each AUV thruster, adds each one to the thrusters vector and waits 7 seconds for the
         *  thrusters to start
         */
        void initializeThrusters();

        /**
         * @brief Function that takes an argument with movement instruction and decides how the thrusters will be activated
         * 
         * @param decision struct with the action decision that will be executed by the AUV and with what power
         */
        void defineAction(Decision decision);

        /**
         * @brief Finish the thrusters
         */
        void finish();
};

#endif //THRUSTERSCTRL_H