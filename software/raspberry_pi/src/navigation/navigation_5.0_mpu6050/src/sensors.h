#ifndef SENSORS_H
#define SENSORS_H

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

struct KalmanFilter {
    double r = .05;  // Ruído do sensor
    double erm = .1; // Erro de medição
    double ve = .5;  // Variância da estimativa
    array<double, 3> ei = {1, 1, 1};  // Estimativa inicial
    array<double, 3> vx = {pow(ve, 2) + r, pow(ve, 2) + r, pow(ve, 2) + r};  // Variância da estimativa extrapolada
    array<double, 3> gk = {0, 0, 0}; // Ganho de Kalman
    array<double, 3> estadoAtual = {0, 0, 0}; // Estado atual
    array<double, 3> variacaoEstadoAtual = {0, 0, 0}; // Variação do estado atual

    KalmanFilter(double r = .05, double erm = .1, double ve = .5);

    ~KalmanFilter();

    /**
     * @brief Aplica filtro de Kalman na aceleração.
     * 
     * Variáveis:
     *  - Ruido = r
     *  - Erro de medição = erm
     *  - Estimativa inicial = ei
     *  - Variância_da_estimativa = ve
     *  - Variância_da_estimativa_extrapolada = vx
     * 
     * @param rawData Dados brutos do sensor.
     * 
     * @return Dados filtrados.
     */
    array<double, 3> filtro_Kalman(array<double, 3> rawData);

    void calc(int i);
};

class Sensors {
    private:
        int i2cFile;                    // Descritor de arquivo para comunicação I2C
        int address;                    // Endereço I2C do sensor

        // Última velocidade calculada para integração simplificada
        array<double, 3> lastVelocity = {0.0, 0.0, 0.0}; // Última velocidade lida
        array<double, 3> acc = {0.0, 0.0, 0.0}; // Última aceleração lida
        array<double, 3> lastAcc = {0.0, 0.0, 0.0}; // Última aceleração lida
        array<double, 3> lastGyro = {0.0, 0.0, 0.0}; // Último giroscópio lido
        array<double, 3> lastOri = {0.0, 0.0, 0.0}; // Última orientação lida

        // Marcação temporal para cálculo de deltaT
        steady_clock::time_point currentTime;
        steady_clock::time_point oldTime;

        // Mutex para garantir segurança em acessos concorrentes
        std::mutex mutexSensors;

        KalmanFilter kalmanFilter; // Filtro de Kalman para suavização dos dados

        // Métodos auxiliares para comunicação I2C
        int16_t readWord(int reg);
        void writeByte(int reg, int data);

    public:
        // Construtor e destrutor
        Sensors(int i2cAddress = 0x68);
        
        ~Sensors();

        // Inicializa comunicação com o sensor
        bool initialize();
        
        /**
         * @brief Obtém o tempo decorrido entre as duas últimas chamadas de updateData.
         */
        duration<double> deltaTime();
        
        /**
         * @brief Atualiza dados internos do sensor (e futuramente de outros sensores).
         */
        void updateData();
        
        // Métodos para obter leituras do sensor
        array<double, 3> getAcc();   // Acelerômetro
        array<double, 3> getGyro();  // Giroscópio
        array<double, 3> getOri();   // Orientação (pitch, roll, yaw)
        array<double, 3> getVel();   // Velocidade estimada via integração
        
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
