#ifndef THRUSTERSCTRL_H
#define THRUSTERSCTRL_H

#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <chrono>
#include <wiringPi.h>
#include "sensors.h"
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

/**
 * @brief Struct representing a decision for thruster action
 */
struct Decision{
    Action action = Action::NONE;
    double value = -1;
};

/**
 * @brief Class representing a single thruster
 */
class Thruster{
    private:
        int pin, stablePower, currentPower;
        int pwmRange = 1000;

    public:
        /**
         * @brief Constructs a new Thruster object.
         * 
         * @param pin Pin connected to the thruster.
         * @param stablePower Power for this thruster to stabilize the AUV (Vx: 0, Vy: 0, Vz: 0).
         */
        Thruster(int pin, int stablePower);

        /**
         * @brief Initializes the thruster.
         * 
         * Sets the pin as output, sets the PWM mode as MS (mark:space), sets the clock divisor
         * (default clock 19.2MHz), sets the PWM value range, and writes 0 to the pin, which corresponds to 1500us.
         */
        void initThruster();

        /**
         * @brief Sets a PWM value into dutycycle value from a percentage value.
         * 
         * @param value A percentage value from -100 to 100.
         */
        void move(double value);

        /**
         * @brief Converts a percentage value to a dutycycle value.
         * 
         * @param value A percentage value from -100 to 100.
         * 
         * @return Corresponding dutycycle value.
         */
        int percentageToDutycycle(double value);

        /**
         * @brief Turns off the thruster.
         * 
         * Sets the duty cycle to 0.
         */
        void finishesThruster();
};

/**
 * @brief Class representing the control of multiple thrusters
 */
class ThrustersControl{
    private:
        vector<Thruster> thrusters;
        unique_ptr<Sensors> sensors;
        thread pidThread;
        // Vector with setpoints values that PID control will use to control the AUV
        // X, Y, Z, Roll, Yaw
        array<double, 5> setPoint = {0, 0, 0, 0, 0};

    public:
        /**
         * @brief Constructs a new ThrustersControl object.
         * 
         * Activates the wiringPi service and initializes each thruster.
         */
        ThrustersControl();

        /**
         * @brief Initializes each AUV thruster.
         * 
         * Creates each AUV thruster, adds each one to the thrusters vector, and waits 7 seconds for the
         * thrusters to start.
         */
        void initializeThrusters();

        /**
         * * @brief Moves the AUV in the X direction.
         * 
         * @param valueDeslocation The distance to move in the X direction (in meters).
         */
        void moveX(double valueDeslocation);

        /**
         * @brief Moves the AUV in the Y direction.
         * 
         * @param valueDeslocation The distance to move in the Y direction (in meters).
         */
        void moveY(double valueDeslocation);

        /**
         * @brief Moves the AUV in the Z direction.
         * 
         * @param valueDeslocation The distance to move in the Z direction (in meters).
         */
        void moveZ(double valueDeslocation);

        /**
         * @brief Rotates the AUV in the Roll direction.
         * 
         * @param valueRotate The angle to rotate in the Roll direction (in rad).
         */
        void rotateRoll(double valueRotate);

        /**
         * @brief Rotates the AUV in the Pitch direction.
         * 
         * @param valueRotate The angle to rotate in the Pitch direction (in rad).
         */
        void rotateYaw(double valueRotate);

        void pidControl();

        void pidX();

        void pidY();

        void pidZ();

        void pidRoll();

        void pidYaw();

        /**
         * @brief Defines the action for the thrusters based on the given decision.
         * 
         * @param decision Struct with the action decision that will be executed by the AUV and with what power.
         */
        void defineAction(Decision decision);

        /**
         * @brief Finish the thrusters.
         */
        void finish();
};

#endif //THRUSTERSCTRL_H