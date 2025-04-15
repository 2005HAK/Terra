#include "thrustersctrl.h"

Thruster::Thruster(int pin, int stablePower, int minPWMus, int maxPWMus) : pin(pin), stablePower(stablePower), minPWMus(minPWMus), maxPWMus(maxPWMus), currentPower(0){
    initThruster();
}

void Thruster::initThruster(){
    pinMode(this->pin, PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS); // Entender e alterar se necessario
    pwmSetClock(19200000 / (FREQUENCY * this->pwmRange));
    pwmSetRange(pwmRange); // 
    pwmWrite(this->pin, percentageToDutycycle(0));
}

void Thruster::move(double value){
    int pwmValue = percentageToDutycycle(value);
    pwmWrite(pin, pwmValue);
}

int Thruster::percentageToDutycycle(double value){
    // ENTENDER POR QUE ISSO SÓ FUNCIONOU COM DOUBLE !!!!!!!!!!!!!!
    double usefulPower = (value > 0 ? (100 - stablePower) : (100 + stablePower)) * (value / 100);
    currentPower = max(-POWER_MAX, min(stablePower + usefulPower, POWER_MAX));

    return (FREQUENCY / 1000000) * (this->minPWMus + ((this->maxPWMus - this->minPWMus) / 2) * (1 + (currentPower / 100))) * pwmRange;
}

int Thruster::getCurrentPower(){
	return this->currentPower;
}

void Thruster::finishesThruster(){
    pwmWrite(this->pin, 0);
}

ThrustersControl::ThrustersControl(Sensors *sensors) : sensors(sensors){
    cout << "Starting thrusters..." << endl;
    
    if(wiringPiSetup() == -1) throw FailedConnectThrusters();

    initializeThrusters();

    pidThread = thread(&ThrustersControl::pidControl, this);

    cout << "Engines thrusters" << endl;
}

void ThrustersControl::initializeThrusters(){
    for(int i = 0; i < PINS.size(); i++){
        if(PINS[i] == 27 || PINS[i] == 22) thrusters.emplace_back(PINS[i], -14, 1100, 1900);
        else thrusters.emplace_back(PINS[i], 0);
    }
    sleep_for(seconds(7));
}
// movimentação baseada nos eixos do AUV
void ThrustersControl::moveX(double valueDeslocation){
    setPoint[0] += valueDeslocation * cos(sensors->getPosition()[4]);
    setPoint[1] += valueDeslocation * sin(sensors->getPosition()[4]);
}

void ThrustersControl::moveY(double valueDeslocation){
    setPoint[1] += valueDeslocation * cos(sensors->getPosition()[4]);
    setPoint[0] += valueDeslocation * -  sin(sensors->getPosition()[4]);
}

void ThrustersControl::moveZ(double valueDeslocation){
    setPoint[2] += valueDeslocation;
}

void ThrustersControl::rotateRoll(double valueDeslocation){
    setPoint[3] += valueDeslocation;
}

void ThrustersControl::rotateYaw(double valueDeslocation){
    setPoint[4] += valueDeslocation;
}

void ThrustersControl::pidControl(){
    while(true){ // Esse true vai dar problema, mas por enquanto é só para testar
        double x = this->setPoint[0] - this->sensors->getPosition()[0];
        double y = this->setPoint[1] - this->sensors->getPosition()[1];
        double d = sqrt(pow(x, 2) + pow(y, 2));
        double yaw = this->sensors->getPosition()[4];
        double theta = acos((x*cos(yaw) + y*sin(yaw)) / d);

        pidX(theta, d);
        pidY(theta, d);
        pidZ();
        pidRoll();
        pidYaw();

        cout << "Fl: " << this->thrusters[0].getCurrentPower() << " %\nFr: " << this->thrusters[1].getCurrentPower() << " %\nBl: " << this->thrusters[4].getCurrentPower() << " %\nBr: " << this->thrusters[3].getCurrentPower() << " %\nMl: " << this->thrusters[5].getCurrentPower() << " %\nMr: " << this->thrusters[2].getCurrentPower() << " %" << endl;

        sleep_for(milliseconds(350)); // Ajustar o tempo de espera para o controle PID
    }
}

// TESTAR SE OS VALORES DE POWER MOVEM OS MOTORES PRO LADO DESEJADO (SE NÃO, INVERTER O SINAL)

void ThrustersControl::pidX(double theta, double d){
    double error = d * cos(theta);
    double power = 0.0;

    double kp = 0.5, kd = 0.01; // Coefficients for PID control

    power = kp * error; // Proportional term
    // FALTA O TERMO DERIVATIVO

    thrusters[0].move(-power);
    thrusters[1].move(-power);
    thrusters[3].move(power);
    thrusters[4].move(power);
}

void ThrustersControl::pidY(double theta, double d){
    double error = d * sin(theta);
    double power = 0.0;

    double kp = 0.5, kd = 0.01; // Coefficients for PID control

    power = kp * error; // Proportional term
    // FALTA O TERMO DERIVATIVO    

    thrusters[0].move(-power);
    thrusters[1].move(power);
    thrusters[3].move(power);
    thrusters[4].move(-power);
}

void ThrustersControl::pidZ(){
    double error = setPoint[2] - sensors->getPosition()[2];
    double power = 0.0;

    double kp = 0.5, kd = 0.01; // Coefficients for PID control

    power = kp * error; // Proportional term
    // FALTA O TERMO DERIVATIVO

    thrusters[2].move(power);
    thrusters[5].move(power);
}

void ThrustersControl::pidRoll(){
    double error = setPoint[3] - sensors->getPosition()[3];
    double power = 0.0;

    double kp = 0.5, ki = 0.1, kd = 0.01; // Coefficients for PID control

    power = kp * error; // Proportional term
    // FALTA OS TERMOS INTEGRAL E DERIVATIVO

    thrusters[2].move(-power);
    thrusters[5].move(power);
}

void ThrustersControl::pidYaw(){
    double error = setPoint[4] - sensors->getPosition()[4];
    double power = 0.0;

    double kp = 0.5, ki = 0.1, kd = 0.01; // Coefficients for PID control

    power = kp * error; // Proportional term
    // FALTA OS TERMOS INTEGRAL E DERIVATIVO

    thrusters[0].move(-power);
    thrusters[1].move(power);
    thrusters[3].move(-power);
    thrusters[4].move(power);
}

void ThrustersControl::finish(){
    cout << "Turning off the thrusters..." << endl;

    for(int i = 0; i < thrusters.size(); i++) thrusters[i].finishesThruster();

    cout << "Thrusters off" << endl;
}
