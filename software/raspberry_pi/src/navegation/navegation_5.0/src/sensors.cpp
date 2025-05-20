#include "sensors.h"

Sensors::Sensors(){
    mavsdk = make_unique<Mavsdk>(Mavsdk::Configuration(1, 1, true));

    ConnectionResult connection_result = mavsdk->add_any_connection("serial:///dev/ttyAMA0:57600");

    if (connection_result != ConnectionResult::Success){ //Colocar isso em um código de erro
        logMessage("Failed to connect: " + connection_result);
    } else logMessage("Connected to Pixhawk");

    sleep_for(seconds(3));
    logMessage("Finding systems...");

    auto systems = mavsdk->systems();

    if(systems.size() > 0) {
        auto system = mavsdk->systems().at(0);
        if(system->is_connected()){
            this->telemetry = make_unique<Telemetry>(system);
            this->mavlink_passthrough = make_unique<MavlinkPassthrough>(system);

            while (!this->telemetry->health().is_accelerometer_calibration_ok ||
            !this->telemetry->health().is_gyrometer_calibration_ok ||
            !this->telemetry->health().is_magnetometer_calibration_ok) {
                logMessage("Waiting for the system to be ready...");
                sleep_for(seconds(1));
            }
        }

        logMessage("System is ready!");
    }else {//Colocar isso em código de erro tbm
        logMessage("Failed to detect system.");
    }

    currentTime = chrono::steady_clock::now();
    oldTime = currentTime;
}

Sensors::~Sensors(){}

void Sensors::initialize(){
    logMessage("Listening messages...");

    if(mavlink_passthrough){
        this->mavlink_passthrough->subscribe_message(MAVLINK_MSG_ID_GLOBAL_POSITION_INT, [this](const mavlink_message_t& message) {
            lock_guard<mutex> lock(mutexSensors);
            mavlink_global_position_int_t imu_data;
            mavlink_msg_global_position_int_decode(&message, &imu_data);

            this->vel[0] = imu_data.vx * CONV_TO_MS;
            this->vel[1] = imu_data.vy * CONV_TO_MS;
            this->vel[2] = imu_data.vz * CONV_TO_MS;
        });

        this->mavlink_passthrough->subscribe_message(MAVLINK_MSG_ID_RAW_IMU, [this](const mavlink_message_t& message) {
            lock_guard<mutex> lock(mutexSensors);
            mavlink_raw_imu_t imu_data;
            mavlink_msg_raw_imu_decode(&message, &imu_data);

            this->acc[0] = imu_data.xacc * CONV_TO_MS2;
            this->acc[1] = imu_data.yacc * CONV_TO_MS2;
            this->acc[2] = imu_data.zacc * CONV_TO_MS2;

            this->gyro[0] = imu_data.xgyro * CONV_TO_RAD;
            this->gyro[1] = imu_data.ygyro * CONV_TO_RAD;
            this->gyro[2] = imu_data.zgyro * CONV_TO_RAD;

            this->tempPixhawk = imu_data.temperature / 100.0;
        });

        this->mavlink_passthrough->subscribe_message(MAVLINK_MSG_ID_ATTITUDE, [this](const mavlink_message_t& message) {
            lock_guard<mutex> lock(mutexSensors);
            mavlink_attitude_t att;
            mavlink_msg_attitude_decode(&message, &att);

            this->ori[0] = att.roll;
            this->ori[1] = att.pitch;
            this->ori[2] = att.yaw; // em rad
        });
    }
}

void Sensors::updateData(){
    // cout << "accx: " << acc[0] << ", accy: " << acc[1] << ", accz: " << acc[2] << endl;

    oldTime = currentTime;
    currentTime = chrono::steady_clock::now();
}

void Sensors::collisionDetect(){
    lock_guard<mutex> lock(mutexSensors);
    if(fabs(this->acc[0]) > ACC_LIMIT || fabs(this->acc[1]) > ACC_LIMIT || fabs(this->acc[2]) > ACC_LIMIT){
        throw CollisionDetected(acc);
    }
}

void Sensors::detectOverheat(){
    lock_guard<mutex> lock(mutexSensors);
    if(this->tempPixhawk > MAX_TEMP_PIXHAWK) throw PixhawkHighTemperature(tempPixhawk);
    if(this->tempRaspberry > MAX_TEMP_RASPBERRY) throw RaspberryHighTemperature(tempRaspberry);
}

array<double, 3> Sensors::getAcc(){
    lock_guard<mutex> lock(mutexSensors);
    return this->acc;
}

array<double, 3> Sensors::getGyro(){
    lock_guard<mutex> lock(mutexSensors);
    return this->gyro;
}

array<double, 3> Sensors::getOri(){
    lock_guard<mutex> lock(mutexSensors);
    return this->ori;
}

array<double, 3> Sensors::getVel(){
    lock_guard<mutex> lock(mutexSensors);
    return this->vel;
}

chrono::duration<double> Sensors::deltaTime(){
    lock_guard<mutex> lock(mutexSensors);
    return this->currentTime - this->oldTime;
}
