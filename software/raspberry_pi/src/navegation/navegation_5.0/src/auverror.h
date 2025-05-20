#ifndef AUVERROR_H
#define AUVERROR_H

#include "utils.h"
#include "logs.h"

/**
 * @brief Class responsible for managing errors that may occur during the AUV operation.
 */
class AUVError : public exception{
    private:
        std::string time;
        std::string type;
        int code;

    public:
        /**
         * @brief Generalization of errors or errors that are not identified.
         * 
         * @param type Type of error identified.
         * @param code Error code.
         */
        AUVError(std::string type = "Unidentidied error", int code = 95);

        /**
         * @brief Saves errors to a log file for later checking.
         */
        void logError();
};

/**
 * @brief Class representing errors that may occur during the initialization of the sensors.
 */
class FailedInitializationSensors : public AUVError{
    public:
        /**
         * @brief Constructor of the FailedInitializationSensors class.
         */
        FailedInitializationSensors();
};

// YOLOCTRL

/**
 * @brief Class representing errors that may occur during the initialization of the Yolo.
 */
class FailedInitializationYolo : public AUVError{
    public:
        /**
         * @brief Constructor of the FailedInitializationYolo class.
         * 
         * @param type Type of error identified.
         * @param code Error code.
         */
        FailedInitializationYolo(std::string type = "Failed to create the yolo object.", int code = 251);
};

/**
 * @brief Class representing errors that may occur during the creation of a socket.
 */
class ErrorCreatingSocket : public FailedInitializationYolo{
    public:
        /**
         * @brief Constructor of the ErrorCreatingSocket class.
         */
        ErrorCreatingSocket();
};

/**
 * @brief Class representing errors that may occur during the binding of a socket.
 */
class ErrorBindingSocket : public FailedInitializationYolo{
    public:
        /**
         * @brief Constructor of the ErrorBindingSocket class.
         */
        ErrorBindingSocket();
};

/**
 * @brief Class representing errors that may occur during the listening of a socket.
 */
class ErrorListening : public FailedInitializationYolo{
    public:
        /**
         * @brief Constructor of the ErrorListening class.
         */
        ErrorListening();
};

/**
 * @brief Class representing errors that may occur during the accepting of a connection.
 */
class ErrorAcceptingConnection : public FailedInitializationYolo{
    public:
        /**
         * @brief Constructor of the ErrorAcceptingConnection class.
         */
        ErrorAcceptingConnection();
};

class DetectionError : public AUVError{
    public:
        /**
         * @brief Constructor of the DetectionError class.
         */
        DetectionError(std::string type = "Failed to detect", int code = 256);
};

class ObjectNotFound : public DetectionError{
    private:
        std::string object;

    public:
        /**
         * @brief Constructor of the ObjectNotFound class.
         * 
         * @param object The object that was not found.
         */
        ObjectNotFound(std::string object = "Unidentified object");
};

// END YOLOCTRL

// SENSORS

class FailedPixhawk : public AUVError{
    public:
        /**
         * @brief Constructor of the FailedPixhawk class.
         * 
         * @param type Type of error identified.
         * @param code Error code.
         */
        FailedPixhawk(std::string type = "Failed in the Pixhawk", int code = 350);
};

class FailedConnectMavsdk : public FailedPixhawk{
    public:
        /**
         * @brief Constructor of the FailedConnectMavsdk class.
         * 
         * @param type Type of error identified.
         * @param code Error code.
         */
        FailedConnectMavsdk();
};

class FailedDetectSystem : public FailedPixhawk{
    public:
        /**
         * @brief Constructor of the FailedFindSystem class.
         * 
         * @param type Type of error identified.
         * @param code Error code.
         */
        FailedDetectSystem();
};

/**
 * @brief Class representing errors that may occur during the initialization of the thrusters.
 */
class CollisionDetected : public AUVError{
    private:
        std::array<double, 3> acceleration;

    public:
        /**
         * @brief Constructor of the CollisionDetected class.
         * 
         * @param acceleration The acceleration values on the x, y, and z axes, respectively.
         */
        CollisionDetected(std::array<double, 3> acceleration);
        
        /**
         * @brief Gets the acceleration values that caused the collision.
         * 
         * @return The acceleration values on the x, y, and z axes, respectively.
         */
        std::array<double, 3> getAcceleration();
};

/**
 * @brief Class representing errors that may occur during the initialization of the thrusters.
 */
class HighTemperatureError : public AUVError{
    private:
    double temperature;
    
    public:
    /**
     * @brief Constructor of the HighTemperatureError class.
     * 
     * @param temperature Temperature value that caused the error.
     * @param type Type of error identified.
     * @param code Error code.
     */
    HighTemperatureError(double temperature = -1, std::string type = "High temperature detected", int code = 95);
};

/**
 * @brief Class representing errors that may occur during the initialization of the thrusters.
 */
class PixhawkHighTemperature : public HighTemperatureError{
    public:
    /**
     * @brief Constructor of the PixhawkHighTemperature class.
     * 
     * @param temperature Temperature value that caused the error.
     */
    PixhawkHighTemperature(double temperature = -1);
};

/**
 * @brief Class representing errors that may occur during the initialization of the thrusters.
 */
class RaspberryHighTemperature : public HighTemperatureError{
    public:
    /**
     * @brief Constructor of the RaspberryHighTemperature class.
     * 
     * @param temperature Temperature value that caused the error.
     */
        RaspberryHighTemperature(double temperature = -1);
};

// END SENSORS

/**
 * @brief Class representing errors that may occur during the initialization of the thrusters.
 */
class FailedConnectThrusters : public AUVError{
    public:
        /**
         * @brief Constructor of the FailedConnectThrusters class.
         */
        FailedConnectThrusters();
};

/**
 * @brief Class representing errors that may occur during the firing of a torpedo.
 */
class FailedFiringTorpedo : public AUVError{
    public:
        /**
         * @brief Constructor of the FailedFiringTorpedo class.
         * 
         * @param type Type of error identified.
         * @param code Error code.
         */
        FailedFiringTorpedo();
};

#endif //AUVERROR_H