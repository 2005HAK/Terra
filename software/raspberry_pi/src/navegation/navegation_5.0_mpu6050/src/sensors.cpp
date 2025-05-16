#include "sensors.h"

void init_MPU6050() {
    wiringPiI2CWriteReg8(fd, 0x6B, 0x00);  // Wake up: PWR_MGMT_1 = 0
    wiringPiI2CWriteReg8(fd, 0x1B, 0x00);  // Gyro config: ±250°/s
    wiringPiI2CWriteReg8(fd, 0x1C, 0x00);  // Accel config: ±2g
}

int16_t read_word_2c(int reg) {
    int16_t high = wiringPiI2CReadReg8(fd, reg);
    int16_t low = wiringPiI2CReadReg8(fd, reg + 1);
    int16_t value = (high << 8) + low;

    if (value >= 0x8000)
        value = -((65535 - value) + 1);

    return value;
}

void getAccel(float& ax, float& ay, float& az) {
    ax = read_word_2c(0x3B) / 16384.0 * 9.80665;
    ay = read_word_2c(0x3D) / 16384.0 * 9.80665;
    az = read_word_2c(0x3F) / 16384.0 * 9.80665;
}

void getGyro(float& gx, float& gy, float& gz) {
    gx = read_word_2c(0x43) / 131.0 * M_PI / 180.0;
    gy = read_word_2c(0x45) / 131.0 * M_PI / 180.0;
    gz = read_word_2c(0x47) / 131.0 * M_PI / 180.0;
}

Sensors::Sensors(){
    // Inicializa o I2C
    fd = wiringPiI2CSetup(MPU);
    if (fd == -1) {
        std::cerr << "Erro ao inicializar o MPU6050!\n";
        return 1;
    }

    init_MPU6050();
    std::cout << "MPU6050 inicializado com sucesso.\n";

    currentTime = chrono::steady_clock::now();
    oldTime = currentTime;
}

Sensors::~Sensors(){}

void Sensors::initialize(){
    cout << "Listening messages..." << endl;
}

void Sensors::updateData(){
    // cout << "accx: " << acc[0] << ", accy: " << acc[1] << ", accz: " << acc[2] << endl;

    getAccel(this->acc[0], this->acc[1], this->acc[2]);
    getGyro(this->gyro[0], this->gyro[1], this->gyro[2]);

    oldTime = currentTime;
    currentTime = chrono::steady_clock::now();
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

chrono::duration<double> Sensors::deltaTime(){
    return this->currentTime - this->oldTime;
}
