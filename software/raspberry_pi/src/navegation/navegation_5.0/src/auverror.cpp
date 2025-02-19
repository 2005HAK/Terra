#include "auverror.h"

AUVError::AUVError(string type = "Unidentidied error", int code = 95) : time(getTime()), type(type), code(code){
    logError();
}

void AUVError::logError(){
    ofstream logFile("auv_errors.log", ios::app);
    if(logFile.is_open()){
        logFile << "[" << this->time << "]" << " (Error " << this->code << "): " << this->type << endl;
        logFile.close();
    } else {
        cout << "Failed to open log file" << endl;
    }
}

FailedInitializationSensors::FailedInitializationSensors() : AUVError("Failed to create the sensors object", 151){}

FailedInitializationYolo ::FailedInitializationYolo() : AUVError("Failed to create the yolo object", 152){}

CollisionDetected::CollisionDetected(array<double, 3> acceleration) : AUVError("Collision detected based in sensor data: {" + to_string(acceleration[0]) +
                ", " + to_string(acceleration[1]) + ", " + to_string(acceleration[2]) + "} m/s²", 301), acceleration(acceleration){}

array<double, 3> CollisionDetected::getAcceleration(){
    return this->acceleration;
}

FailedConnectThrusters::FailedConnectThrusters() : AUVError("Failed to connect thrusters", 441){}

HighTemperatureError::HighTemperatureError(double temperature = -1, string type = "High temperature detected", int code = 95) : AUVError(type, code), temperature(temperature){}

PixhawkHighTemperature::PixhawkHighTemperature(double temperature = -1) : HighTemperatureError(temperature, "Pixhawk temperature above threshold " + to_string(temperature) + "ºC", 332){}

RaspberryHighTemperature::RaspberryHighTemperature(double temperature = -1) : HighTemperatureError(temperature, "Raspberry temperature above threshold " + to_string(temperature) + "ºC", 141){}
