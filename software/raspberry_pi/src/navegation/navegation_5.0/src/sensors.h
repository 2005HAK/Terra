#ifndef SENSORS_H
#define SENSORS_H

#include <array>
#include <iostream>
#include "mavsdk/mavsdk.h"
#include "mavsdk/plugins/telemetry/telemetry.h"
#include "mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h"
#include "auverror.h"

using namespace mavsdk;

// Convertions
const double CONV_TO_MS = .01;       // convert cm/s to m/s 
const double CONV_TO_MS2 = 0.00980665;  // convert mG to m/s²
const double CONV_TO_RAD = .001;     // convert mrad/s to rad/s
const double CONV_TO_UT = .1;        // convert mgauss to µT

//Acceleration limit consider a collision
const double ACC_LIMIT = 15;

// Critical temperatures - !!!Determinar quais são as temperaturas maximas!!!
const double MAX_TEMP_PIXHAWK = 60;
const double MAX_TEMP_RASPBERRY = 70;
const double MAX_TEMP_JETSON = 60;

class Sensors{
    private:
        array<double, 3> acc = {0, 0, 0};       // {x, y, z}
        array<double, 3> gyro = {0, 0, 0};      // {x, y, z}
        array<double, 3> mag = {0, 0, 0};       // {x, y, z}
        array<double, 3> vel = {0, 0, 0};       // {x, y, z}
        double tempPixhawk = 0.0;               // ºC
        double tempRaspberry = 0.0;             // ºC
        steady_clock::time_point currentTime;
        steady_clock::time_point oldTime;
        unique_ptr<Telemetry> telemetry;
        unique_ptr<MavlinkPassthrough> mavlink_passthrough;
        unique_ptr<Mavsdk> mavsdk;

    public:
        /**
         * @brief Initializes the Sensors class, establishing a connection to Pixhawk and setting initial values for acceleration, gyro, magnetometer,
         *  velocity, and temperature sensors.
         */
        Sensors();
        
        ~Sensors();

        void initialize();

        /**
         * @brief Continuously updates sensor data, fetching new information from Pixhawk and ****temperature sensors (fazer essa parte)****
         */
        void updateData();
            

        /**
         * @brief Detects whether the AUV has crashed based on acceleration data from Pixhawk
         */
        void collisionDetect();

        /**
         * @brief Checks whether the systems temperature is safe **** Terminar!! ****
         */
        void detectOverheat();

        array<double, 3> getAcc();

        array<double, 3> getGyro();

        array<double, 3> getMag();

        array<double, 3> getVel();

        chrono::duration<double> deltaTime();
};

#endif //SENSORS_H
