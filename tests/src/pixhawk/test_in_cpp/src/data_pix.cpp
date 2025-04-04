#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace mavsdk;
using namespace std;
using namespace this_thread;
using namespace chrono;


int main() {
    Mavsdk mavsdk{Mavsdk::Configuration(1,1,true)};

    // Conexão com a Pixhawk via USB
    ConnectionResult connection_result = mavsdk.add_any_connection("serial:///dev/ttyACM0:115200");
    if (connection_result != ConnectionResult::Success) {
        std::cerr << "Failed to connect: " << connection_result << std::endl;
        return 1;
    }

    std::cout << "Connected to Pixhawk via USB!" << std::endl;

    sleep_for(seconds(2));

    auto systems = mavsdk.systems();

    if (systems.empty()) {
        std::cerr << "No systems detected." << std::endl;
        return 1;
    }
    
    std::cout << "Number of systems detected: " << systems.size() << std::endl;

    // Obtém o sistema
    auto system = mavsdk.systems().at(0);

    if (!system) {
        std::cerr << "Failed to detect system." << std::endl;
        return 1;
    }

    Telemetry telemetry{system};

    // Verifica se a conexão está ativa
    while (!telemetry.health().is_accelerometer_calibration_ok ||
           !telemetry.health().is_gyrometer_calibration_ok || 
           !telemetry.health().is_magnetometer_calibration_ok) {
        std::cout << "Waiting for the system to be ready..." << std::endl;
        sleep_for(seconds(1));
    }
    
    std::cout << "System is ready!" << std::endl;

    MavlinkPassthrough mavlink_passthrough{system};

    cout << "Listening messages..." << endl;

    mavlink_passthrough.subscribe_message(MAVLINK_MSG_ID_GLOBAL_POSITION_INT, [](const mavlink_message_t& message) {
        mavlink_global_position_int_t imu_data;
        mavlink_msg_global_position_int_decode(&message, &imu_data);

        cout << "GLOBAL_POSITION_INT:" << endl;
        cout << "Vel X: " << imu_data.vx / 100.0 << " m/s" << endl;
        cout << "Vel Y: " << imu_data.vy / 100.0 << " m/s" << endl;
        cout << "Vel Z: " << imu_data.vz / 100.0 << " m/s" << endl;
    });

    mavlink_passthrough.subscribe_message(MAVLINK_MSG_ID_RAW_IMU, [](const mavlink_message_t& message) {
        mavlink_raw_imu_t imu_data;
        mavlink_msg_raw_imu_decode(&message, &imu_data);

        cout << "\nRAW_IMU Data:" << endl;
        cout << "Accel X: " << imu_data.xacc * 9.80665 / 1000 << " m/s²" << endl;
        cout << "Accel Y: " << imu_data.yacc * 9.80665 / 1000 << " m/s²" << endl;
        cout << "Accel Z: " << imu_data.zacc * 9.80665 / 1000 << " m/s²" << endl;

        cout << "Gyro X: " << imu_data.xgyro * 0.001 << " rad/s" << endl;
        cout << "Gyro Y: " << imu_data.ygyro * 0.001 << " rad/s" << endl;
        cout << "Gyro Z: " << imu_data.zgyro * 0.001 << " rad/s" << endl;
        
        cout << "Mag X: " << imu_data.xmag * 0.1 << " µT" << endl;
        cout << "Mag Y: " << imu_data.ymag * 0.1 << " µT" << endl;
        cout << "Mag Z: " << imu_data.zmag * 0.1 << " µT" << endl;

        cout << "Temp: " << imu_data.temperature / 100.0 << "ºC" << endl;
    });

    // Loop infinito para manter o programa rodando
    while (true) {
        sleep_for(milliseconds(100));
    }


    return 0;
}
