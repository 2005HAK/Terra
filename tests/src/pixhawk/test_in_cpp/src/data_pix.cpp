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

vector<float> vel_ned = {0, 0, 0};
vector<float> vel_local = {0, 0, 0};

float last_yaw = 0.0;

int main() {
    Mavsdk mavsdk{Mavsdk::Configuration(1,1,true)};
    ConnectionResult connection_result = mavsdk.add_any_connection("serial:///dev/ttyACM1:115200");

    if (connection_result != ConnectionResult::Success) {
        cerr << "Failed to connect: " << connection_result << endl;
        return 1;
    }

    cout << "Connected to Pixhawk via USB!" << endl;
    sleep_for(seconds(2));

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
           !telemetry.health().is_gyrometer_calibration_ok || 
           !telemetry.health().is_magnetometer_calibration_ok) {
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
    });

    // Subscreve à mensagem de posição global para pegar as velocidades
    mavlink_passthrough.subscribe_message(MAVLINK_MSG_ID_GLOBAL_POSITION_INT, [](const mavlink_message_t& message) {
        mavlink_global_position_int_t pos;
        mavlink_msg_global_position_int_decode(&message, &pos);

        vel_ned[0] = pos.vx / 100.0f; // norte
        vel_ned[1] = pos.vy / 100.0f; // leste
        vel_ned[2] = pos.vz / 100.0f; // descendo

        // Rotação de NED -> local (com x = frente do AUV)
        float cos_yaw = cos(last_yaw);
        float sin_yaw = sin(last_yaw);

        vel_local[0] =  vel_ned[0] * cos_yaw + vel_ned[1] * sin_yaw;  // x (frente)
        vel_local[1] = -vel_ned[0] * sin_yaw + vel_ned[1] * cos_yaw;  // y (direita)
        vel_local[2] =  vel_ned[2];                                   // z (descida)
    });
    
    // Loop infinito para imprimir as velocidades locais
    while (true) {
        cout << fixed;
        cout.precision(2);
        cout << "Velocidade LOCAL (em relação ao AUV):" << endl;
        cout << "X (frente):   " << vel_ned[0] << " m/s" << endl;
        cout << "Y (direita):  " << vel_ned[1] << " m/s" << endl;
        cout << "Z (descendo): " << vel_ned[2] << " m/s" << endl;
        cout << "Yaw atual:    " << last_yaw * 180.0 / M_PI << " graus" << endl;
        cout << "-------------------------------------" << endl;

        sleep_for(milliseconds(333));
    }

    return 0;
}
