#include <array>
#include <chrono>
#include <thread>
#include <iostream>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include "auv_error.cpp"

using namespace std;
using namespace chrono;
using namespace mavsdk;
using namespace this_thread;

// Convertions
double CONV_TO_MS = .01;       // convert cm/s to m/s 
double CONV_TO_MS2 = 9806.65;  // convert mG to m/s²
double CONV_TO_RAD = .001;     // convert mrad/s to rad/s
double CONV_TO_µT = .1;        // convert mgauss to µT

//Acceleration limit consider a collision
double ACC_LIMIT = 15;

// Critical temperatures - !!!Determinar quais são as temperaturas maximas!!!
double MAX_TEMP_PIXHAWK = 60;
double MAX_TEMP_RASPBERRY = 70;
double MAX_TEMP_JETSON = 60;

class Sensors{
    private:
        array<double, 3> acc = {0, 0, 0};       // {x, y, z}
        array<double, 3> gyro = {0, 0, 0};      // {x, y, z}
        array<double, 3> mag = {0, 0, 0};       // {x, y, z}
        array<double, 3> vel = {0, 0, 0};       // {x, y, z}
        double tempPixhawk;                     // ºC
        double tempRaspberry;                   // ºC
        steady_clock::time_point currentTime;
        steady_clock::time_point oldTime;
        Telemetry *telemetry;
        MavlinkPassthrough *mavlink_passthrough;

    public:
        /**
         * @brief Initializes the Sensors class, establishing a connection to Pixhawk and setting initial values for acceleration, gyro, magnetometer,
         *  velocity, and temperature sensors.
         */
        Sensors(){
            Mavsdk mavsdk{Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};

            ConnectionResult connection_result = mavsdk.add_any_connection("serial:///dev/ttyACM0:115200");

            if (connection_result != ConnectionResult::Success){ //Colocar isso em um código de erro
                cout << "Failed to connect: " << connection_result << endl;
            } else cout << "Connected to Pixhawk" << endl;

            sleep_for(seconds(2));

            auto systems = mavsdk.systems();

            if(!systems.empty()) {
                auto system = mavsdk.systems().at(0);
                
                this->telemetry = new Telemetry{system};
                this->mavlink_passthrough = new MavlinkPassthrough{system};

                while (!this->telemetry->health().is_accelerometer_calibration_ok ||
                    !this->telemetry->health().is_gyrometer_calibration_ok || 
                    !this->telemetry->health().is_magnetometer_calibration_ok) {
                    std::cout << "Waiting for the system to be ready..." << std::endl;
                    sleep_for(seconds(1));
                }
    
                std::cout << "System is ready!" << std::endl;
            }else {//Colocar isso em código de erro tbm
                std::cerr << "Failed to detect system." << std::endl;
            }
        }

        /**
         * @brief Continuously updates sensor data, fetching new information from Pixhawk and ****temperature sensors (fazer essa parte)****
         */
        void updateData(){
            cout << "Listening messages..." << endl;

            this->mavlink_passthrough->subscribe_message(MAVLINK_MSG_ID_GLOBAL_POSITION_INT, [this](const mavlink_message_t& message) {
                mavlink_global_position_int_t imu_data;
                mavlink_msg_global_position_int_decode(&message, &imu_data);

                this->vel[0] = imu_data.vx * CONV_TO_MS;
                this->vel[1] = imu_data.vy * CONV_TO_MS;
                this->vel[2] = imu_data.vz * CONV_TO_MS;
            });

            this->mavlink_passthrough->subscribe_message(MAVLINK_MSG_ID_RAW_IMU, [this](const mavlink_message_t& message) {
                mavlink_raw_imu_t imu_data;
                mavlink_msg_raw_imu_decode(&message, &imu_data);

                this->acc[0] = imu_data.xacc * CONV_TO_MS2;
                this->acc[1] = imu_data.yacc * CONV_TO_MS2;
                this->acc[2] = imu_data.zacc * CONV_TO_MS2;

                this->gyro[0] = imu_data.xgyro * CONV_TO_RAD;
                this->gyro[1] = imu_data.ygyro * CONV_TO_RAD;
                this->gyro[2] = imu_data.zgyro * CONV_TO_RAD;

                this->mag[0] = imu_data.xmag * CONV_TO_µT;
                this->mag[1] = imu_data.ymag * CONV_TO_µT;
                this->mag[2] = imu_data.zmag * CONV_TO_µT;

                this->tempPixhawk = imu_data.temperature / 100.0;
            });

            while (1) sleep_for(milliseconds(100));
        }

        /**
         * @brief Detects whether the AUV has crashed based on acceleration data from Pixhawk
         */
        void collisionDetect(){
            if(fabs(this->acc[0]) > ACC_LIMIT || fabs(this->acc[1]) > ACC_LIMIT || fabs(this->acc[2]) > ACC_LIMIT){
                throw CollisionDetected(acc);
            }
        }

        /**
         * @brief Checks whether the systems temperature is safe **** Terminar!! ****
         */
        void detectOverheat(){
            if(this->tempPixhawk > MAX_TEMP_PIXHAWK) throw PixhawkHighTemperature(tempPixhawk);
            if(this->tempRaspberry > MAX_TEMP_RASPBERRY) throw RaspberryHighTemperature(tempRaspberry);
        }

        void finish(){
            delete this->telemetry;
            delete this->mavlink_passthrough;
        }

        array<double, 3> getAcc(){
            return this->acc;
        }

        array<double, 3> getGyro(){
            return this->gyro;
        }

        array<double, 3> getMag(){
            return this->mag;
        }

        array<double, 3> getVel(){
            return this->vel;
        }
};