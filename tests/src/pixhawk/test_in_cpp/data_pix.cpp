#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/action/action.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace mavsdk;
using namespace std;
using namespace this_thread;
using namespace chrono;


int main() {
    Mavsdk mavsdk{Mavsdk::Configuration{Mavsdk::ComponentType::CompanionComputer}};

    // Conexão com a Pixhawk via USB
    ConnectionResult connection_result = mavsdk.add_any_connection("serial:///dev/ttyACM1:115200");
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

    sleep_for(seconds(5));

    Telemetry telemetry{system};

    cout << telemetry.health() << endl;

    // Verifica se a conexão está ativa
    while (!telemetry.health_all_ok()) {
        std::cout << "Waiting for the system to be ready..." << std::endl;
        sleep_for(seconds(1));
    }
    
    std::cout << "System is ready!" << std::endl;

    // Lê dados da IMU
    while(1){
        telemetry.subscribe_imu([](Telemetry::Imu imu_data) {
            std::cout << "Accel NED: " 
                    << "North: " << imu_data.acceleration_frd.forward_m_s2 << " m/s², "
                    << "East: " << imu_data.acceleration_frd.right_m_s2 << " m/s², "
                    << "Down: " << imu_data.acceleration_frd.down_m_s2 << " m/s²" << std::endl;
        });
    }
    

    return 0;
}
