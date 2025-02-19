#ifndef AUVERROR_H
#define AUVERROR_H

#include <iostream>
#include <fstream>
#include <array>
#include "utils.h"

class AUVError : public exception{
    private:
        string time;
        string type;
        int code;

    public:
        /**
         * @brief Generalization of errors or errors that are not identified
         * 
         * @param type Type of error identified
         */
        AUVError(string type = "Unidentidied error", int code = 95);

        /**
         * @brief Saves errors to a log file for later checking
         */
        void logError();
};

class FailedInitializationSensors : public AUVError{
    public:
        FailedInitializationSensors();
};

class FailedInitializationYolo : public AUVError{
    public:
        FailedInitializationYolo();
};

class CollisionDetected : public AUVError{
    private:
        array<double, 3> acceleration;

    public:
        /**
         * @brief Detects AUV collisions
         */
        CollisionDetected(array<double, 3> acceleration);
        
        array<double, 3> getAcceleration();
};

class FailedConnectThrusters : public AUVError{
    public:
        FailedConnectThrusters();
};

class HighTemperatureError : public AUVError{
    private:
        double temperature;

    public:
        /**
         * @brief Generalization of temperature error or unidentified temperature problem
         */
        HighTemperatureError(double temperature = -1, string type = "High temperature detected", int code = 95);
};

class PixhawkHighTemperature : public HighTemperatureError{
    public:
        /**
         * @brief High temperature at Pixhawk
         */
        PixhawkHighTemperature(double temperature = -1);
};

class RaspberryHighTemperature : public HighTemperatureError{
    public:
        /**
         * @brief High temperature at Raspberry
         */
        RaspberryHighTemperature(double temperature = -1);
};

#endif //AUVERROR_H