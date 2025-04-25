#include "sensors.h"

Sensors::Sensors(){
    mavsdk = make_unique<Mavsdk>(Mavsdk::Configuration(1, 1, true));

    ConnectionResult connection_result = mavsdk->add_any_connection("serial:///dev/ttyAMA0:115200");

    if (connection_result != ConnectionResult::Success){ //Colocar isso em um código de erro
        cout << "Failed to connect: " << connection_result << endl;
    } else cout << "Connected to Pixhawk" << endl;

    sleep_for(seconds(3));
    cout << "Finding systems..." << endl;

    auto systems = mavsdk->systems();

    if(systems.size() > 0) {
        auto system = mavsdk->systems().at(0);
        if(system->is_connected()){
            this->telemetry = make_unique<Telemetry>(system);
            this->mavlink_passthrough = make_unique<MavlinkPassthrough>(system);

            while (!this->telemetry->health().is_accelerometer_calibration_ok ||
            !this->telemetry->health().is_gyrometer_calibration_ok ||
            !this->telemetry->health().is_magnetometer_calibration_ok) {
                std::cout << "Waiting for the system to be ready..." << std::endl;
                sleep_for(seconds(1));
            }
        }

        std::cout << "System is ready!" << std::endl;
    }else {//Colocar isso em código de erro tbm
        std::cerr << "Failed to detect system." << std::endl;
    }
}

Sensors::~Sensors(){}

void Sensors::initialize(){
    cout << "Listening messages..." << endl;

    if(mavlink_passthrough){
        this->mavlink_passthrough->subscribe_message(MAVLINK_MSG_ID_GLOBAL_POSITION_INT, [this](const mavlink_message_t& message) {
            mavlink_global_position_int_t imu_data;
            mavlink_msg_global_position_int_decode(&message, &imu_data);

            double alpha = 0.1; // Fator de suavização

            this->oldTimeV = this->currentTimeV;
            this->currentTimeV = imu_data.time_boot_ms;

            this->velOld[0] = this->vel[0];
            this->velOld[1] = this->vel[1];
            this->velOld[2] = this->vel[2];
            this->vel[0] = (alpha * imu_data.vx * CONV_TO_MS + (1 - alpha) * this->vel[0]);
            this->vel[1] = (alpha * imu_data.vy * CONV_TO_MS + (1 - alpha) * this->vel[1]);
            this->vel[2] = (alpha * imu_data.vz * CONV_TO_MS + (1 - alpha) * this->vel[2]);
            
            this->position[0] += (((this->vel[0] + this->velOld[0]) * this->deltaTimeV() * 10e-3) / 2); // X
            this->position[1] += (((this->vel[1] + this->velOld[1]) * this->deltaTimeV() * 10e-3) / 2); // Y
            this->position[2] += (((this->vel[2] + this->velOld[2]) * this->deltaTimeV() * 10e-3) / 2); // Z
        });

        this->mavlink_passthrough->subscribe_message(MAVLINK_MSG_ID_RAW_IMU, [this](const mavlink_message_t& message) {
            mavlink_raw_imu_t imu_data;
            mavlink_msg_raw_imu_decode(&message, &imu_data);

            this->oldTimeA = this->currentTimeA;
            this->currentTimeA = imu_data.time_usec;

            this->accOld[0] = this->acc[0];
            this->accOld[1] = this->acc[1];
            this->accOld[2] = this->acc[2];
            this->acc[0] = imu_data.xacc * CONV_TO_MS2;
            this->acc[1] = imu_data.yacc * CONV_TO_MS2;
            this->acc[2] = imu_data.zacc * CONV_TO_MS2;

            this->gyro[0] = imu_data.xgyro * CONV_TO_RAD;
            this->gyro[1] = imu_data.ygyro * CONV_TO_RAD;
            this->gyro[2] = imu_data.zgyro * CONV_TO_RAD;
            
            this->magOld[0] = this->mag[0];
            this->magOld[1] = this->mag[1];
            this->magOld[2] = this->mag[2];
            this->mag[0] = imu_data.xmag * CONV_TO_UT;
            this->mag[1] = imu_data.ymag * CONV_TO_UT;
            this->mag[2] = imu_data.zmag * CONV_TO_UT;

            this->tempPixhawk = imu_data.temperature / 100.0;
        });
        
        // Obtenção dos dados do giroscópio de forma mais precisa
        this->mavlink_passthrough->subscribe_message(MAVLINK_MSG_ID_ATTITUDE, [this](const mavlink_message_t& message) {
            mavlink_attitude_t imu_data;
            mavlink_msg_attitude_decode(&message, &imu_data);

            this->position[3] = imu_data.roll;
            this->position[4] = imu_data.yaw;
            this->position[5] = imu_data.pitch;
        });
    }
}

void Sensors::updateData(){
    cout << "X: " << this->position[0] << " m\nY: " << this->position[1] << " m\nZ: " << this->position[2] << " m\nRoll: " << this->position[3] << " rad\nYaw: " << this->position[4] << " rad" << endl;
}

void Sensors::collisionDetect(){
    if(fabs(this->acc[0]) > ACC_LIMIT || fabs(this->acc[1]) > ACC_LIMIT || fabs(this->acc[2]) > ACC_LIMIT){
        throw CollisionDetected(acc);
    }
}

void Sensors::detectOverheat(){
    if(this->tempPixhawk > MAX_TEMP_PIXHAWK) throw PixhawkHighTemperature(tempPixhawk);
    if(this->tempRaspberry > MAX_TEMP_RASPBERRY) throw RaspberryHighTemperature(tempRaspberry);
}

array<double, 3> Sensors::getAcc(){
    return this->acc;
}

array<double, 3> Sensors::getGyro(){
    return this->gyro;
}

array<double, 3> Sensors::getMag(){
    return this->mag;
}

array<double, 3> Sensors::getVel(){
    return this->vel;
}

array<double, 6> Sensors::getPosition(){
    return this->position;
}

// Mudar no código principal depois, vou mudar o retorno
uint32_t Sensors::deltaTimeV(){
    return this->currentTimeV - this->oldTimeV;
}

uint32_t Sensors::deltaTimeA(){
    return this->currentTimeA - this->oldTimeA;
}
