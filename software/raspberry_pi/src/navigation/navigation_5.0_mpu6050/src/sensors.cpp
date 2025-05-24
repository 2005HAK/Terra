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

// Obtém valores do acelerômetro e converte para 'g'
array<double, 3> Sensors::getAcc() {
    double ax = readWord(0x3B) / 16384.0;
    double ay = readWord(0x3D) / 16384.0;
    double az = readWord(0x3F) / 16384.0;
    return {ax, ay, az};
}

// Obtém valores do giroscópio em graus/s
array<double, 3> Sensors::getGyro() {
    double gx = readWord(0x43) / 131.0;
    double gy = readWord(0x45) / 131.0;
    double gz = readWord(0x47) / 131.0;
    return {gx, gy, gz};
}

// Calcula orientação: yaw, pitch e roll
array<double, 3> Sensors::getOri() {
    auto acc = getAcc();
    double ax = acc[0];
    double ay = acc[1];
    double az = acc[2];

    // Cálculo do pitch e roll baseado no acelerômetro
    double pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / M_PI;
    double roll = atan2(ay, az) * 180.0 / M_PI;

    // Sem magnetômetro, yaw não é confiável
    double yaw = 0.0;

    return {yaw, pitch, roll};
}

// Estima a velocidade via integração simples da aceleração
array<double, 3> Sensors::getVel() {
    static auto lastTime = steady_clock::now();

    auto now = steady_clock::now();
    double deltaT = duration_cast<duration<double>>(now - lastTime).count();
    lastTime = now;

    auto acc = getAcc();

    for (int i = 0; i < 3; i++) {
        // Integração: v = v0 + a * deltaT
        lastVelocity[i] += acc[i] * 9.81 * deltaT;  // Convertendo 'g' para m/s²
    }

    return lastVelocity;
}

// Atualiza os timestamps para controle de tempo
void Sensors::updateData() {
    std::lock_guard<std::mutex> lock(mutexSensors);
    oldTime = currentTime;
    currentTime = steady_clock::now();
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
