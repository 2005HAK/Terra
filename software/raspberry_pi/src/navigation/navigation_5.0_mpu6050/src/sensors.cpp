#include "sensors.h"

using namespace std::chrono;

// Construtor: define endereço e inicializa o descritor como inválido
Sensors::Sensors(int i2cAddress) : address(i2cAddress), i2cFile(-1) {}

// Destrutor: garante que o arquivo I2C será fechado
Sensors::~Sensors() {
    if (i2cFile >= 0) {
        close(i2cFile);
    }
}

// Inicializa o sensor MPU6050 na interface I2C
bool Sensors::initialize() {
    const char *filename = "/dev/i2c-1";
    
    if ((i2cFile = open(filename, O_RDWR)) < 0) {
        std::cerr << "Failed to open I2C bus\n";
        return false;
    }
    
    if (ioctl(i2cFile, I2C_SLAVE, address) < 0) {
        std::cerr << "Failed to acquire bus access\n";
        return false;
    }
    
    // Desativa modo sleep do MPU6050
    writeByte(0x6B, 0);
    return true;
}

// Escreve um byte em um registrador do sensor
void Sensors::writeByte(int reg, int data) {
    char buf[2] = { (char)reg, (char)data };
    if (write(i2cFile, buf, 2) != 2) {
        std::cerr << "Failed to write to the I2C bus\n";
    }
}

// Lê uma palavra (2 bytes) de um registrador do sensor
int16_t Sensors::readWord(int reg) {
    char buf[1] = { (char)reg };
    if (write(i2cFile, buf, 1) != 1) {
        std::cerr << "Failed to set register for read\n";
    }
    
    char data[2];
    if (read(i2cFile, data, 2) != 2) {
        std::cerr << "Failed to read from I2C\n";
    }
    
    return (int16_t)((data[0] << 8) | data[1]);
}

// Atualiza os timestamps para controle de tempo
void Sensors::updateData() {
    std::lock_guard<std::mutex> lock(mutexSensors);

    // Atualiza os dados do sensor
    lastAcc = acc;
    acc = {(readWord(0x3B) / 16384.0) * 9.81, (readWord(0x3D) / 16384.0) * 9.81, (readWord(0x3F) / 16384.0) * 9.81};
    lastGyro = {(readWord(0x43) / 131.0), (readWord(0x45) / 131.0), (readWord(0x47) / 131.0)};
    lastOri = {atan2(this->lastAcc[1], this->lastAcc[2]) * 180.0 / M_PI, atan2(-this->lastAcc[0], sqrt(this->lastAcc[1] * this->lastAcc[1] + this->lastAcc[2] * this->lastAcc[2])) * 180.0 / M_PI, 0.0};

    double deltaT = duration_cast<duration<double>>(this->deltaTime()).count();

    for (int i = 0; i < 3; i++) {
        // Integração: v = v0 + a * deltaT
        lastVelocity[i] += lastAcc[i] * deltaT;
    }

    oldTime = currentTime;
    currentTime = steady_clock::now();

    array<double, 3> vel = getVel();
    
    logMessage("Velocidade: " + std::to_string(vel[0]) + ", " + std::to_string(vel[1]) + ", " + std::to_string(vel[2]));
}

// Obtém valores do acelerômetro e converte para m/s²
array<double, 3> Sensors::getAcc() {
    return this->lastAcc;
}

// Obtém valores do giroscópio em graus/s
array<double, 3> Sensors::getGyro() {
    return this->lastGyro;
}

// Calcula orientação: yaw, pitch e roll
array<double, 3> Sensors::getOri() {
    return this->lastOri;
}

// Estima a velocidade via integração simples da aceleração
array<double, 3> Sensors::getVel() {
    return this->lastVelocity;
}


// Placeholder: lógica de detecção de colisão ainda não implementada
void Sensors::collisionDetect() {
    // TODO: Implementar detecção de colisão
}

// Placeholder: lógica de detecção de sobreaquecimento ainda não implementada
void Sensors::detectOverheat() {
    // TODO: Implementar detecção de sobreaquecimento
}

// Retorna o delta de tempo entre as duas últimas atualizações
duration<double> Sensors::deltaTime() {
    return currentTime - oldTime;
}
