#include "auverror.h"

AUVError::AUVError(std::string type, int code) : time(getTime()), type(type), code(code){
    logError();
}

void AUVError::logError(){
    std::ofstream logFile("auv_errors.log", std::ios::app);
    std::string msg = "[" + this->time + "]" + " (Error " + to_string(this->code) + "): " + this->type;

    if(logFile.is_open()){
        logFile << msg << endl;
        logFile.close();
    } else {
        std::cout << "Failed to open log file: auv_errors.log" << std::endl;
    }
}

FailedInitializationSensors::FailedInitializationSensors() : AUVError("Failed to create the sensors object.", 151){}

// YOLOCTRL

FailedInitializationYolo ::FailedInitializationYolo(std::string type, int code) : AUVError(type, code){}

ErrorCreatingSocket::ErrorCreatingSocket() : FailedInitializationYolo("Failed to create the socket.", 252){}

ErrorBindingSocket::ErrorBindingSocket() : FailedInitializationYolo("Failed to bind the socket.", 253){}

ErrorListening::ErrorListening() : FailedInitializationYolo("Failed to listen to the socket.", 254){}

ErrorAcceptingConnection::ErrorAcceptingConnection() : FailedInitializationYolo("Failed to accept the connection.", 255){}

DetectionError::DetectionError(std::string type, int code) : AUVError(type, code){}

ObjectNotFound::ObjectNotFound(std::string object) : DetectionError("Failed to detected the object: " + object, 257), object(object){}

// END YOLOCTRL

CollisionDetected::CollisionDetected(std::array<double, 3> acceleration) : AUVError("Collision detected based in sensor data: {" + to_string(acceleration[0]) +
                ", " + to_string(acceleration[1]) + ", " + to_string(acceleration[2]) + "} m/s²", 301), acceleration(acceleration){}

std::array<double, 3> CollisionDetected::getAcceleration(){
    return this->acceleration;
}

FailedConnectThrusters::FailedConnectThrusters() : AUVError("Failed to connect thrusters", 441){}

HighTemperatureError::HighTemperatureError(double temperature, std::string type, int code) : AUVError(type, code), temperature(temperature){}

PixhawkHighTemperature::PixhawkHighTemperature(double temperature) : HighTemperatureError(temperature, "Pixhawk temperature above threshold " + to_string(temperature) + "ºC", 332){}

RaspberryHighTemperature::RaspberryHighTemperature(double temperature) : HighTemperatureError(temperature, "Raspberry temperature above threshold " + to_string(temperature) + "ºC", 141){}

FailedFiringTorpedo::FailedFiringTorpedo() : AUVError("Failed to fire torpedo", 820){}