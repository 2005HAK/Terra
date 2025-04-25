#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

using namespace mavsdk;
using namespace std;
using namespace this_thread;
using namespace chrono;

const double CONV_TO_MS = .01;       // convert cm/s to m/s 
const double CONV_TO_MS2 = 0.00980665;  // convert mG to m/s²

vector<float> velOld = {0, 0, 0};
vector<float> vel = {0, 0, 0};
vector<float> accOld = {0, 0, 0};
vector<float> acc = {0, 0, 0};
vector<float> position = {0, 0, 0};

uint32_t oldTimeV = 0; // Time of the last update ms
uint32_t currentTimeV = 0; // Time of the current update ms
uint32_t deltaTime = 0; // Time of the current update ms
bool firstTime = true; // Flag to indicate if it's the first time the velocity is updated

float last_yaw = 0.0;
float last_roll = 0.0;
float last_pitch = 0.0;

int main() {
    Mavsdk mavsdk{Mavsdk::Configuration(1,1,true)};
    ConnectionResult connection_result = mavsdk.add_any_connection("serial:///dev/ttyACM0:115200");

    if (connection_result != ConnectionResult::Success) {
        cerr << "Failed to connect: " << connection_result << endl;
        return 1;
    }

    cout << "Connected to Pixhawk via USB!" << endl;
    
    sleep_for(seconds(1));

    auto systems = mavsdk.systems();
    if (systems.empty()) {
        cerr << "No systems detected." << endl;
        return 1;
    }

    auto system = systems.at(0);
    if (!system) {
        cerr << "Failed to detect system." << endl;
        return 1;
    }
    
    
    Telemetry telemetry{system};
    while (!telemetry.health().is_accelerometer_calibration_ok ||
           !telemetry.health().is_gyrometer_calibration_ok){
           //|| !telemetry.health().is_magnetometer_calibration_ok) {
        cout << "Waiting for the system to be ready..." << endl;
        sleep_for(seconds(1));
    }
    
    cout << "System is ready!" << endl;

    MavlinkPassthrough mavlink_passthrough{system};

    // Subscreve ao ATTITUDE para obter o yaw (em radianos)
    mavlink_passthrough.subscribe_message(MAVLINK_MSG_ID_ATTITUDE, [](const mavlink_message_t& message) {
        mavlink_attitude_t att;
        mavlink_msg_attitude_decode(&message, &att);

        last_yaw = att.yaw; // em rad
        last_pitch = att.pitch;
        last_roll = att.roll;
    });
    /*
    mavlink_passthrough.subscribe_message(MAVLINK_MSG_ID_RAW_IMU, [](const mavlink_message_t& message) {
        mavlink_raw_imu_t imu;
        mavlink_msg_raw_imu_decode(&message, &imu);

        oldTimeV = currentTimeV;
        currentTimeV = imu.time_usec;

        if(firstTime){
            oldTimeV = currentTimeV;
            firstTime = false;
        }

        deltaTime = (currentTimeV - oldTimeV) / 1000;

        accOld[0] = acc[0];        
        accOld[1] = acc[1];        
        accOld[2] = acc[2];

        cout << imu.xacc << endl;

        acc[0] = imu.xacc * CONV_TO_MS2;
        acc[1] = imu.yacc * CONV_TO_MS2;
        acc[2] = imu.zacc * CONV_TO_MS2;
        
        vel[0] += ((acc[0] + accOld[0]) * (deltaTime / 1000)) / 2;
        vel[1] += ((acc[1] + accOld[1]) * (deltaTime / 1000)) / 2;
        vel[2] += ((acc[2] + accOld[2]) * (deltaTime / 1000)) / 2;
    });*/

    // Subscreve à mensagem de posição global para pegar as velocidades
    mavlink_passthrough.subscribe_message(MAVLINK_MSG_ID_LOCAL_POSITION_NED, [](const mavlink_message_t& message) {
        mavlink_local_position_ned_t imu_data;
        mavlink_msg_local_position_ned_decode(&message, &imu_data);

        double alpha = 0.07; // Fator de suavização

        oldTimeV = currentTimeV;
        currentTimeV = imu_data.time_boot_ms;

        if(firstTime){
            oldTimeV = currentTimeV;
            firstTime = false;
        }

        deltaTime = currentTimeV - oldTimeV;

        velOld[0] = vel[0];
        velOld[1] = vel[1];
        velOld[2] = vel[2];
        vel[0] = imu_data.vx;
        vel[1] = imu_data.vy;
        vel[2] = imu_data.vz;
        //vel[0] = (alpha * imu_data.vx * CONV_TO_MS + (1 - alpha) * vel[0]) < 0.008 && (alpha * imu_data.vx * CONV_TO_MS + (1 - alpha) * vel[0]) > -0.008 ? 0 : (alpha * imu_data.vx * CONV_TO_MS + (1 - alpha) * vel[0]);
        //vel[1] = (alpha * imu_data.vy * CONV_TO_MS + (1 - alpha) * vel[1]) < 0.008 && (alpha * imu_data.vy * CONV_TO_MS + (1 - alpha) * vel[1]) > -0.008 ? 0 : (alpha * imu_data.vy * CONV_TO_MS + (1 - alpha) * vel[1]);
        //vel[2] = (alpha * imu_data.vz * CONV_TO_MS + (1 - alpha) * vel[2]) < 0.02 && (alpha * imu_data.vz * CONV_TO_MS + (1 - alpha) * vel[2]) > -0.02 ? 0 : (alpha * imu_data.vz * CONV_TO_MS + (1 - alpha) * vel[2]);
        
        //position[0] += (((vel[0] + velOld[0]) * (deltaTime / 1000)) / 2); // X
        //position[1] += (((vel[1] + velOld[1]) * (deltaTime / 1000)) / 2); // Y
        //position[2] += (((vel[2] + velOld[2]) * (deltaTime / 1000)) / 2); // Z

        position[0] = imu_data.x;
        position[1] = imu_data.y;
        position[2] = imu_data.z;
    });
    
    // Loop infinito para imprimir as velocidades locais
    while (true) {
        cout << fixed;
        cout << "Velocidade:" << endl;
        cout << "deltaTime: " << deltaTime << " ms" << endl;
        cout << "X: " << vel[0] << " m/s" << endl;
        cout << "Y: " << vel[1] << " m/s" << endl;
        cout << "Z: " << vel[2] << " m/s" << endl;
        cout << "Posição:" << endl;
        cout << "X: " << position[0] << " m" << endl;
        cout << "Y: " << position[1] << " m" << endl;
        cout << "Z: " << position[2] << " m" << endl;
        cout << "Yaw: " << last_yaw * 180 / M_PI << " degrees" << endl;
        cout << "-------------------------------------" << endl;

        sleep_for(milliseconds(33));
    }

    return 0;
}
