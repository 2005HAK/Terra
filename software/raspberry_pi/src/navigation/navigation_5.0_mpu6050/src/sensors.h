#ifndef SENSORS_H
#define SENSORS_H

#include <wiringPiI2C.h>
#include <Eigen/Dense>
#include <cmath>
#include <iostream>
#include "utils.h"
/**
 * @brief      Classe para leitura e filtragem de dados do sensor MPU6050 usando filtro de Kalman.
 * @details    Realiza leitura de aceleração e giroscópio, calcula orientação e aplica filtro de Kalman nos eixos pitch e roll.
 */
class Sensors {
public:
    /**
     * @brief      Construtor da classe
     * @details    Inicializa variáveis com zero e configura o I2C.
     */
    Sensors();

    /**
     * @brief      Inicializa o sensor MPU6050.
     * @details    Configura o sensor para começar a comunicar via I2C.
     */
    void initialize();

    /**
     * @brief      Calibra o sensor MPU6050.
     * @details    Calcula os offsets do acelerômetro e giroscópio com base em várias amostras.
     * @param      samples Número de amostras para a calibração (padrão: 1000).
     */
    void calibrate(int samples = 1000);

    /**
     * @brief      Atualiza leituras do sensor e aplica o filtro de Kalman.
     * @details    Faz a leitura dos dados brutos, converte para unidades físicas e atualiza a estimativa com Kalman.
     */
    void update();

    /**
     * @brief      Retorna aceleração linear no eixo X (inercial).
     * @details    Usa orientação estimada para compensar gravidade.
     * @return     Aceleração linear no eixo X (m/s²)
     */
    float getLinearAccelerationX();

    /**
     * @brief      Retorna aceleração linear no eixo Y (inercial).
     * @details    Usa orientação estimada para compensar gravidade.
     * @return     Aceleração linear no eixo Y (m/s²)
     */
    float getLinearAccelerationY();

    /**
     * @brief      Retorna aceleração linear no eixo Z (inercial).
     * @details    Usa orientação estimada para compensar gravidade.
     * @return     Aceleração linear no eixo Z (m/s²)
     */
    float getLinearAccelerationZ();

    /**
     * @brief      Retorna velocidade angular no eixo X.
     * @return     Velocidade angular em rad/s
     */
    float getGyroX();

    /**
     * @brief      Retorna velocidade angular no eixo Y.
     * @return     Velocidade angular em rad/s
     */
    float getGyroY();

    /**
     * @brief      Retorna velocidade angular no eixo Z.
     * @return     Velocidade angular em rad/s
     */
    float getGyroZ();

    /**
     * @brief      Retorna o ângulo de rotação estimado no eixo Roll.
     * @return     Ângulo em graus
     */
    float getRoll();

    /**
     * @brief      Retorna o ângulo de inclinação estimado no eixo Pitch.
     * @return     Ângulo em graus
     */
    float getPitch();

    /**
     * @brief      Retorna o ângulo de guinada estimado no eixo Yaw.
     * @return     Ângulo em graus
     */
    float getYaw();

    array<double, 3> Sensors::getAcc();
    array<double, 3> Sensors::getGyro();
    array<double, 3> Sensors::getOri();
    array<double, 3> Sensors::getVel();

private:
    int fd; // File descriptor para I2C
    float gyroX, gyroY, gyroZ;
    float accX, accY, accZ;
    float accXOffset, accYOffset, accZOffset; // Offsets do acelerômetro
    float gyroXOffset, gyroYOffset, gyroZOffset; // Offsets do giroscópio
    float accXInertial, accYInertial, accZInertial; // Acelerações lineares
    float angleRoll, anglePitch;
    float estRoll, estPitch;
    float yaw; // Ângulo de guinada
    unsigned long currentTime, previousTime; // Tempos para cálculo do passo dinâmico
    float dt; // Passo de tempo dinâmico (em segundos)

    // Kalman - Pitch
    Eigen::Matrix<float, 2, 2> F, P, Q, I; // Matrizes comuns
    Eigen::Matrix<float, 2, 1> S, G, K; // Estado, controle e ganho
    Eigen::Matrix<float, 1, 2> H; // Observação
    Eigen::Matrix<float, 1, 1> R, Z; // Covariância de medição e medição
    Eigen::Matrix<float, 2, 1> X1; // Estado estimado (pitch)
    Eigen::Matrix<float, 2, 2> P1; // Covariância do erro (pitch)

    // Kalman - Roll
    Eigen::Matrix<float, 2, 1> X2; // Estado estimado (roll)
    Eigen::Matrix<float, 2, 2> P2; // Covariância do erro (roll)
};

#endif //SENSORS_H
