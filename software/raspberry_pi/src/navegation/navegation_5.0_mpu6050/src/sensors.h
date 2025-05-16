#ifndef SENSORS_H
#define SENSORS_H

#include <array>
#include <iostream>
#include <cmath>
#include <wiringPiI2C.h>
#include "mpu6050.h"
#include "mavsdk/mavsdk.h"
#include "mavsdk/plugins/telemetry/telemetry.h"
#include "mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h"
#include "auverror.h"

using namespace mavsdk;

// Endereço do MPU6050 no barramento I2C
#define MPU 0x68

// Variáveis globais
int fd;  // file descriptor para comunicação I2C

// Convertions constants
const double CONV_TO_MS = .01;       // convert cm/s to m/s 
const double CONV_TO_MS2 = 0.00980665;  // convert mG to m/s²
const double CONV_TO_RAD = .001;     // convert mrad/s to rad/s
const double CONV_TO_UT = .1;        // convert mgauss to µT

//Acceleration limit consider a collision
const double ACC_LIMIT = 15;

// Critical temperatures - !!!Determinar quais são as temperaturas maximas!!!
const double MAX_TEMP_PIXHAWK = 60;      // Maximum temperature for Pixhawk (ºC)
const double MAX_TEMP_RASPBERRY = 70;    // Maximum temperature for Raspberry (ºC)
const double MAX_TEMP_JETSON = 60;       // Maximum temperature for Jetson (ºC)

// Inicializa o MPU6050
void init_MPU6050();

// Função auxiliar para ler registradores de 16 bits (MSB primeiro)
int16_t read_word_2c(int reg);

// Lê os valores brutos do acelerômetro e converte para m/s²
void getAccel(float& ax, float& ay, float& az);

// Lê os valores brutos do giroscópio e converte para rad/s
void getGyro(float& gx, float& gy, float& gz);

/**
 * @brief Class responsible for managing the sensors of the AUV, including the Pixhawk sensors and temperature sensors.
 */
class Sensors{
    private:
        array<double, 3> acc = {0, 0, 0};       // Acceleration {x, y, z}
        array<double, 3> gyro = {0, 0, 0};      // Gyroscope {x, y, z}
        array<double, 3> vel = {0, 0, 0};       // Velocity {x, y, z}
        double tempPixhawk = 0.0;               // Temperature of Pixhawk ºC
        double tempRaspberry = 0.0;             // Temperature of Raspberry ºC
        double tempJetson = 0.0;                // Temperature of Jetson ºC
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
        
        /**
         * @brief Destroys the Sensors class.
         */
        ~Sensors();

        /**
         * @brief Initializes the sensores, listening to the Pixhawk messages.
         */
        void initialize();

        /**
         * @brief Continuously updates sensor data, fetching new information from Pixhawk and ****temperature sensors (fazer essa parte)****
         */
        void updateData();

        /**
         * @brief Detects whether the AUV has collided with an object.
         */
        void collisionDetect();

        /**
         * @brief Checks whether the system's temperature is above the maximum allowed.
         */
        void detectOverheat();

        /**
         * @brief Gets the current acceleration values.
         * 
         * @return The acceleration values on the x, y, and z axes, respectively.
         */
        array<double, 3> getAcc();

        /**
         * @brief Gets the current gyroscope values.
         * 
         * @return The gyroscope values on the x, y, and z axes, respectively.
         */
        array<double, 3> getGyro();

        /**
         * @brief Gets the current magnetometer values.
         * 
         * @return The magnetometer values on the x, y, and z axes, respectively.
         */
        array<double, 3> getMag();

        /**
         * @brief Gets the current velocity values.
         * 
         * @return The velocity values on the x, y, and z axes, respectively.
         */
        array<double, 3> getVel();

        /**
         * @brief Gets the time elapsed between the last two calls to updateData.
         * 
         * @return The time elapsed between the last two calls to updateData.
         */
        chrono::duration<double> deltaTime();
};

#endif //SENSORS_H
