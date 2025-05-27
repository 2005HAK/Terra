#include "auverror.h"

AUVError::AUVError(string type, int code) : time(getTime()), type(type), code(code){
    logError();
}

void AUVError::logError(){
    ofstream logFile("auv_errors.log", ios::app);
    string msg = "[" + this->time + "]" + " (Error " + to_string(this->code) + "): " + this->type;

    if(logFile.is_open()){
        logFile << msg << endl;
        logFile.close();
    } else {
        logMessage("Failed to open log file: auv_errors.log");
    }
}

FailedInitializationSensors::FailedInitializationSensors() : AUVError("Failed to create the sensors object.", 151){}

// YOLOCTRL

FailedInitializationYolo ::FailedInitializationYolo(string type, int code) : AUVError(type, code){}

ErrorCreatingSocket::ErrorCreatingSocket() : FailedInitializationYolo("Failed to create the socket.", 252){}

ErrorBindingSocket::ErrorBindingSocket() : FailedInitializationYolo("Failed to bind the socket.", 253){}

ErrorListening::ErrorListening() : FailedInitializationYolo("Failed to listen to the socket.", 254){}

ErrorAcceptingConnection::ErrorAcceptingConnection() : FailedInitializationYolo("Failed to accept the connection.", 255){}

DetectionError::DetectionError(string type, int code) : AUVError(type, code){}

ObjectNotFound::ObjectNotFound(string object) : DetectionError("Failed to detected the object: " + object, 257), object(object){}

// END YOLOCTRL

// SENSORS

FailedPixhawk::FailedPixhawk(string type, int code) : AUVError(type, code){}

FailedConnectMavsdk::FailedConnectMavsdk() : FailedPixhawk("Failed to connect to Mavsdk", 351){}

FailedDetectSystem::FailedDetectSystem() : FailedPixhawk("Failed to find system", 352){}

HighTemperatureError::HighTemperatureError(double temperature, string type, int code) : AUVError(type, code), temperature(temperature){}

PixhawkHighTemperature::PixhawkHighTemperature(double temperature) : HighTemperatureError(temperature, "Pixhawk temperature above threshold " + to_string(temperature) + "ºC", 332){}

RaspberryHighTemperature::RaspberryHighTemperature(double temperature) : HighTemperatureError(temperature, "Raspberry temperature above threshold " + to_string(temperature) + "ºC", 141){}

// END SENSORS

CollisionDetected::CollisionDetected(array<double, 3> acceleration) : AUVError("Collision detected based in sensor data: {" + to_string(acceleration[0]) +
                ", " + to_string(acceleration[1]) + ", " + to_string(acceleration[2]) + "} m/s²", 301), acceleration(acceleration){}

array<double, 3> CollisionDetected::getAcceleration(){
    return this->acceleration;
}

FailedConnectWiringPi::FailedConnectWiringPi() : AUVError("Failed to connect WiringPi", 152){}

FailedFiringTorpedo::FailedFiringTorpedo() : AUVError("Failed to fire torpedo", 820){}