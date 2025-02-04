#pragma once
#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
#include <array>

using namespace std;
using namespace chrono;

/**
 * Gets the time in string format
 * 
 * @return strNowTime
 */
string getTime(){
    auto now = system_clock::now();
    time_t nowTime = system_clock::to_time_t(now);
    string strNowTime = ctime(&nowTime);
    strNowTime.pop_back();
    return strNowTime;
}

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
        AUVError(string type = "Unidentidied error", int code = 95) : time(getTime()), type(type), code(code){
            logError();
        }

        /**
         * @brief Saves errors to a log file for later checking
         */
        void logError(){
            ofstream logFile("auv_errors.log", ios::app);
            if(logFile.is_open()){
                logFile << "[" << this->time << "]" << " (Error " << this->code << "): " << this->type << endl;
                logFile.close();
            } else {
                cout << "Failed to open log file" << endl;
            }
        }
};

class CollisionDetected : public AUVError{
    private:
        array<double, 3> acceleration;

    public:
        /**
         * @brief Detects AUV collisions
         */
        CollisionDetected(array<double, 3> acceleration) : AUVError("Collision detected based in sensor data: {" + to_string(acceleration[0]) +
                          ", " + to_string(acceleration[1]) + ", " + to_string(acceleration[2]) + "} m/s²", 301), acceleration(acceleration){}
        
        array<double, 3> getAcceleration(){
            return this->acceleration;
        }
};

class FailedConnectThrusters : public AUVError{
    public:
        FailedConnectThrusters() : AUVError("Failed to connect thrusters", 441){}
};

class HighTemperatureError : public AUVError{
    private:
        double temperature;

    public:
        /**
         * @brief Generalization of temperature error or unidentified temperature problem
         */
        HighTemperatureError(double temperature = -1, string type = "High temperature detected", int code = 95) : AUVError(type, code), temperature(temperature){}
};

class PixhawkHighTemperature : public HighTemperatureError{
    public:
        /**
         * @brief High temperature at Pixhawk
         */
        PixhawkHighTemperature(double temperature = -1) : HighTemperatureError(temperature, "Pixhawk temperature above threshold " + to_string(temperature) + "ºC", 332){}
};

class RaspberryHighTemperature : public HighTemperatureError{
    public:
        /**
         * @brief High temperature at Raspberry
         */
        RaspberryHighTemperature(double temperature = -1) : HighTemperatureError(temperature, "Raspberry temperature above threshold " + to_string(temperature) + "ºC", 141){}
};
