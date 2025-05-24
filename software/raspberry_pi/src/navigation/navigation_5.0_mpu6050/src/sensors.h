#ifndef SENSORS_H
#define SENSORS_H

#include <array>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <cmath>
#include <chrono>
#include <mutex>
#include "auverror.h"

using std::array;
using namespace std::chrono;

class Sensors {
private:
    int i2cFile;                    // Descritor de arquivo para comunicação I2C
    int address;                    // Endereço I2C do sensor

    // Última velocidade calculada para integração simplificada
    array<double, 3> lastVelocity = {0.0, 0.0, 0.0};

    // Marcação temporal para cálculo de deltaT
    steady_clock::time_point currentTime;
    steady_clock::time_point oldTime;

    // Mutex para garantir segurança em acessos concorrentes
    std::mutex mutexSensors;

    // Métodos auxiliares para comunicação I2C
    int16_t readWord(int reg);
    void writeByte(int reg, int data);

public:
    // Construtor e destrutor
    Sensors(int i2cAddress = 0x68);
    ~Sensors();

    // Inicializa comunicação com o sensor
    bool initialize();

    // Métodos para obter leituras do sensor
    array<double, 3> getAcc();   // Acelerômetro
    array<double, 3> getGyro();  // Giroscópio
    array<double, 3> getOri();   // Orientação (pitch, roll, yaw)
    array<double, 3> getVel();   // Velocidade estimada via integração

    /**
     * @brief Obtém o tempo decorrido entre as duas últimas chamadas de updateData.
     */
    duration<double> deltaTime();

    /**
     * @brief Atualiza dados internos do sensor (e futuramente de outros sensores).
     */
    void updateData();

    /**
     * @brief Detecta colisões do AUV.
     */
    void collisionDetect();

    /**
     * @brief Verifica sobreaquecimento do sistema.
     */
    void detectOverheat();
};

#endif
