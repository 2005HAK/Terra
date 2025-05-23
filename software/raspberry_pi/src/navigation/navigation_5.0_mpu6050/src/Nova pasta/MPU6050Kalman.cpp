#include "Sensors.h"
#include <wiringPi.h>
#include <sys/time.h>

/**
 * @brief      Construtor da classe Sensors
 * @details    Inicializa variáveis com zero e configura o I2C
 */
Sensors::Sensors() {
    gyroX = gyroY = gyroZ = 0.0f;
    accX = accY = accZ = 0.0f;
    accXOffset = accYOffset = accZOffset = 0.0f;
    gyroXOffset = gyroYOffset = gyroZOffset = 0.0f;
    accXInertial = accYInertial = accZInertial = 0.0f;
    angleRoll = anglePitch = 0.0f;
    estRoll = estPitch = 0.0f;
    yaw = 0.0f;
    currentTime = 0;
    previousTime = 0;
    dt = 0.004f; // Valor inicial, será atualizado dinamicamente

    // Inicialização do filtro de Kalman
    F << 1, dt,
         0, 1;
    G << 0.5 * dt * dt,
         dt;
    H << 1, 0;
    I << 1, 0,
         0, 1;
    Q = G * G.transpose() * 10 * 10;
    R << 30 * 30;
    P << 0, 0,
         0, 0;
    S << 0,
         0;
    X1 << 0, 0;
    P1 << 0, 0,
          0, 0;
    X2 << 0, 0;
    P2 << 0, 0,
          0, 0;

    // Inicializa o WiringPi
    wiringPiSetup();
    fd = wiringPiI2CSetup(0x68); // Endereço I2C do MPU6050
    if (fd < 0) {
        std::cerr << "Erro ao inicializar I2C" << std::endl;
    }
}

/**
 * @brief      Inicializa o sensor MPU6050
 * @details    Configura o sensor para operação, desligando o modo de suspensão
 */
void Sensors::initialize() {
    wiringPiI2CWriteReg8(fd, 0x6B, 0); // Desativa modo sleep
    delayMicroseconds(100000); // 100ms
    //calibra
    calibrate(1000); // Calibra o sensor com 1000 amostras
}

/**
 * @brief      Calibra o sensor MPU6050
 * @details    Calcula os offsets do acelerômetro e giroscópio com base em várias amostras
 * @param      samples Número de amostras para a calibração
 */
void Sensors::calibrate(int samples) {
    float sumAccX = 0.0f, sumAccY = 0.0f, sumAccZ = 0.0f;
    float sumGyroX = 0.0f, sumGyroY = 0.0f, sumGyroZ = 0.0f;

    for (int i = 0; i < samples; i++) {
        wiringPiI2CWriteReg8(fd, 0x3B, 0); // Início dos registros
        int16_t rawAccX = wiringPiI2CReadReg16(fd, 0x3B);
        int16_t rawAccY = wiringPiI2CReadReg16(fd, 0x3D);
        int16_t rawAccZ = wiringPiI2CReadReg16(fd, 0x3F);
        int16_t rawGyroX = wiringPiI2CReadReg16(fd, 0x43);
        int16_t rawGyroY = wiringPiI2CReadReg16(fd, 0x45);
        int16_t rawGyroZ = wiringPiI2CReadReg16(fd, 0x47);

        sumAccX += (float)rawAccX / 4096.0f;
        sumAccY += (float)rawAccY / 4096.0f;
        sumAccZ += (float)rawAccZ / 4096.0f;
        sumGyroX += (float)rawGyroX / 131.0f;
        sumGyroY += (float)rawGyroY / 131.0f;
        sumGyroZ += (float)rawGyroZ / 131.0f;

        delayMicroseconds(2000); // 2ms
    }

    // Calcula os offsets
    accXOffset = sumAccX / samples;
    accYOffset = sumAccY / samples;
    accZOffset = sumAccZ / samples - 1.0f; // Compensa 1g
    gyroXOffset = sumGyroX / samples;
    gyroYOffset = sumGyroY / samples;
    gyroZOffset = sumGyroZ / samples;
}

/**
 * @brief      Atualiza as leituras e executa o filtro de Kalman
 * @details    Lê os dados brutos, calcula ângulos, aplica o filtro de Kalman e obtém acelerações inerciais
 */
void Sensors::update() {
    // Calcula o passo de tempo dinâmico
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    previousTime = currentTime;
    currentTime = tv.tv_sec * 1000000 + tv.tv_usec;
    dt = (currentTime - previousTime) / 1000000.0f; // Em segundos

    wiringPiI2CWriteReg8(fd, 0x3B, 0); // Início dos registros
    int16_t rawAccX = wiringPiI2CReadReg16(fd, 0x3B);
    int16_t rawAccY = wiringPiI2CReadReg16(fd, 0x3D);
    int16_t rawAccZ = wiringPiI2CReadReg16(fd, 0x3F);
    int16_t rawGyroX = wiringPiI2CReadReg16(fd, 0x43);
    int16_t rawGyroY = wiringPiI2CReadReg16(fd, 0x45);
    int16_t rawGyroZ = wiringPiI2CReadReg16(fd, 0x47);

    // Aplica os offsets de calibração
    accX = ((float)rawAccX / 4096.0f) - accXOffset;
    accY = ((float)rawAccY / 4096.0f) - accYOffset;
    accZ = ((float)rawAccZ / 4096.0f) - accZOffset;
    gyroX = ((float)rawGyroX / 131.0f) - gyroXOffset;
    gyroY = ((float)rawGyroY / 131.0f) - gyroYOffset;
    gyroZ = ((float)rawGyroZ / 131.0f) - gyroZOffset;

    // Calcula ângulos a partir do acelerômetro
    angleRoll = atan2(accY, sqrt(accX * accX + accZ * accZ)) * 180.0f / M_PI;
    anglePitch = -atan2(accX, sqrt(accY * accY + accZ * accZ)) * 180.0f / M_PI;

    // Atualiza as matrizes do filtro de Kalman
    F << 1, dt,
         0, 1;
    G << 0.5 * dt * dt,
         dt;
    Q = G * G.transpose() * 5 * 5;

    // Filtro de Kalman - Pitch
    Eigen::Matrix<float, 1, 1> Z1;
    Z1 << anglePitch;
    H << 1, 0;
    I << 1, 0,
         0, 1;
    R << 3;
    X1 = F * X1 + G * gyroY;
    P1 = F * P1 * F.transpose() + Q;
    Eigen::Matrix<float, 1, 1> Y1 = Z1 - H * X1;
    Eigen::Matrix<float, 1, 1> S1 = H * P1 * H.transpose() + R;
    K = P1 * H.transpose() * S1.inverse();
    X1 = X1 + K * Y1;
    P1 = (I - K * H) * P1;
    estPitch = X1(0);

    // Filtro de Kalman - Roll
    Eigen::Matrix<float, 1, 1> Z2;
    Z2 << angleRoll;
    X2 = F * X2 + G * gyroX;
    P2 = F * P2 * F.transpose() + Q;
    Eigen::Matrix<float, 1, 1> Y2 = Z2 - H * X2;
    Eigen::Matrix<float, 1, 1> S2 = H * P2 * H.transpose() + R;
    K = P2 * H.transpose() * S2.inverse();
    X2 = X2 + K * Y2;
    P2 = (I - K * H) * P2;
    estRoll = X2(0);

    // Calcula o yaw por integração do giroscópio
    yaw += gyroZ * dt;

    // Calcula acelerações inerciais (m/s²)
    float rollRad = estRoll * M_PI / 180.0f;
    float pitchRad = estPitch * M_PI / 180.0f;
    accXInertial = (accX - sin(pitchRad)) * 9.81f;
    accYInertial = (accY + sin(rollRad) * cos(pitchRad)) * 9.81f;
    accZInertial = (accZ - cos(rollRad) * cos(pitchRad)) * 9.81f;
}

/**
 * @brief      Retorna a aceleração linear no eixo X em m/s²
 * @return     Aceleração linear X
 */
float Sensors::getLinearAccelerationX() {

    return accXInertial;
}

/**
 * @brief      Retorna a aceleração linear no eixo Y em m/s²
 * @return     Aceleração linear Y
 */
float Sensors::getLinearAccelerationY() {
    return accYInertial;
}

/**
 * @brief      Retorna a aceleração linear no eixo Z em m/s²
 * @return     Aceleração linear Z
 */
float Sensors::getLinearAccelerationZ() {
    return accZInertial;
}

array<double, 3> Sensors::getAcc(){
    array<double, 3> acc;
    acc[0] = accX;
    acc[1] = accY;
    acc[2] = accZ;
    return acc;
}

array<double, 3> Sensors::getGyro(){
    array<double, 3> gyro;
    gyro[0] = gyroX;
    gyro[1] = gyroY;
    gyro[2] = gyroZ;
    return gyro;
}
array<double, 3> Sensors::getOri(){
    array<double, 3> ori;
    ori[0] = estRoll;
    ori[1] = estPitch;
    ori[2] = yaw;
    return ori;
}
array<double, 3> Sensors::getVel(){
    array<double, 3> vel;
    vel[0] = accXInertial;
    vel[1] = accYInertial;
    vel[2] = accZInertial;
    return vel;
}


/**
 * @brief      Retorna a velocidade angular no eixo X em rad/s
 * @return     Velocidade angular X
 */
float Sensors::getGyroX() {
    return gyroX * M_PI / 180.0f;
}

/**
 * @brief      Retorna a velocidade angular no eixo Y em rad/s
 * @return     Velocidade angular Y
 */
float Sensors::getGyroY() {
    return gyroY * M_PI / 180.0f;
}

/**
 * @brief      Retorna a velocidade angular no eixo Z em rad/s
 * @return     Velocidade angular Z
 */
float Sensors::getGyroZ() {
    return gyroZ * M_PI / 180.0f;
}

/**
 * @brief      Retorna o ângulo estimado de rolagem (Roll) em graus
 * @return     Ângulo Roll
 */
float Sensors::getRoll() {
    return estRoll;
}

/**
 * @brief      Retorna o ângulo estimado de inclinação (Pitch) em graus
 * @return     Ângulo Pitch
 */
float Sensors::getPitch() {
    return estPitch;
}

/**
 * @brief      Retorna o ângulo estimado de guinada (Yaw) em graus
 * @return     Ângulo Yaw
 */
float Sensors::getYaw() {
    return yaw;
}