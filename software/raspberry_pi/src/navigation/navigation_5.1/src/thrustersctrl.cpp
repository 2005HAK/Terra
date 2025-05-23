#include "thrustersctrl.h"

Thruster::Thruster(int pin, int stablePower) : pin(pin), stablePower(stablePower), currentPower(0){
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
    int minPWMus = 1100, maxPWMus = 1900;

    double usefulPower = (value > 0 ? (100 - stablePower) : (100 + stablePower)) * (value / 100);
    currentPower = max(-POWER_MAX, min(stablePower + usefulPower, POWER_MAX));

    return (FREQUENCY / 1000000) * (minPWMus + ((maxPWMus - minPWMus) / 2) * (1 + (currentPower / 100))) * pwmRange;
}

void Thruster::finishesThruster(){
    pwmWrite(this->pin, 0);
}

ThrustersControl::ThrustersControl(unique_ptr<Sensors> sensors) : sensors(sensors){
    cout << "Starting thrusters..." << endl;
    
    if(wiringPiSetup() == -1) throw FailedConnectThrusters();

    initializeThrusters();

    pidThread = thread(&ThrustersControl::pidControl, this);

    cout << "Engines thrusters" << endl;
}

void ThrustersControl::initializeThrusters(){
    for(int i = 0; i < PINS.size(); i++){
        if(PINS[i] == 27 || PINS[i] == 22) thrusters.emplace_back(PINS[i], -14);
        else thrusters.emplace_back(PINS[i], 0);
    }
    sleep_for(seconds(7));
}

void ThrustersControl::moveX(double valueDeslocation){
    setPoint[0] += valueDeslocation;
}

void ThrustersControl::moveY(double valueDeslocation){
    setPoint[1] += valueDeslocation;
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
        pidX();
        pidY();
        pidZ();
        pidRoll();
        pidYaw();

        sleep_for(milliseconds(350)); // Ajustar o tempo de espera para o controle PID
    }
    
}

// TESTAR SE OS VALORES DE POWER MOVEM OS MOTORES PRO LADO DESEJADO (SE NÃO, INVERTER O SINAL)

void ThrustersControl::pidX(){
    double error = setPoint[0] - sensors->getPosition()[0];
    double power = 0.0;

    double kp = 0.5, ki = 0.1, kd = 0.01; // Coefficients for PID control

    power = kp * error; // Proportional term
    // FALTA OS TERMOS INTEGRAL E DERIVATIVO

    thrusters[0].move(-power);
    thrusters[1].move(-power);
    thrusters[3].move(power);
    thrusters[4].move(power);
}

void ThrustersControl::pidY(){
    double error = setPoint[1] - sensors->getPosition()[1];
    double power = 0.0;

    double kp = 0.5, ki = 0.1, kd = 0.01; // Coefficients for PID control

    power = kp * error; // Proportional term
    // FALTA OS TERMOS INTEGRAL E DERIVATIVO    

    thrusters[0].move(-power);
    thrusters[1].move(power);
    thrusters[3].move(power);
    thrusters[4].move(-power);
}

void ThrustersControl::pidZ(){
    double error = setPoint[2] - sensors->getPosition()[2];
    double power = 0.0;

    double kp = 0.5, ki = 0.1, kd = 0.01; // Coefficients for PID control

    power = kp * error; // Proportional term
    // FALTA OS TERMOS INTEGRAL E DERIVATIVO

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

/* ISSO VAI SAIR
void ThrustersControl::defineAction(Decision decision){
    cout << "Action: " << actionToString(decision.action) << ", Power: " << decision.value << endl;
    stabilizeHori = true;
    stabilizeVert = true;

    switch (decision.action){
        case Action::UP:
            stabilizeVert = false;
            thrusters[2].move(decision.value);
            thrusters[5].move(decision.value);
            break;
        case Action::DOWN:
            stabilizeVert = false;
            thrusters[2].move(-decision.value);
            thrusters[5].move(-decision.value);
            break;
        case Action::FORWARD:
            stabilizeHori = false
            thrusters[0].move(-decision.value);
            thrusters[1].move(-decision.value);
            thrusters[3].move(decision.value);
            thrusters[4].move(decision.value);
            break;
        case Action::BACKWARD:
            stabilizeHori = false;
            thrusters[0].move(decision.value);
            thrusters[1].move(decision.value);
            thrusters[3].move(-decision.value);
            thrusters[4].move(-decision.value);
            break;
        case Action::RIGHT:
            stabilizeHori = false;
            thrusters[0].move(-decision.value);
            thrusters[1].move(decision.value);
            thrusters[3].move(decision.value);
            thrusters[4].move(-decision.value);
            break;
        case Action::LEFT:
            stabilizeHori = false;
            thrusters[0].move(decision.value);
            thrusters[1].move(-decision.value);
            thrusters[3].move(-decision.value);
            thrusters[4].move(decision.value);
            break;
        case Action::TURNRIGHT:
            stabilizeHori = false;
            thrusters[0].move(-decision.value);
            thrusters[1].move(decision.value);
            thrusters[3].move(-decision.value);
            thrusters[4].move(decision.value);
            break;
        case Action::TURNLEFT:
            stabilizeHori = false;
            thrusters[0].move(decision.value);
            thrusters[1].move(-decision.value);
            thrusters[3].move(decision.value);
            thrusters[4].move(-decision.value);
            break;
        default:
            thrusters[0].move(0);
            thrusters[1].move(0);
            thrusters[3].move(0);
            thrusters[4].move(0);
            break;
    }
    sleep_for(milliseconds(350));
}

*/

bool ThrustersControl::getStabilizeVert(){
    return stabilizeVert;
}

bool ThrustersControl::getStabilizeHori(){
    return stabilizeHori;
}

void ThrustersControl::finish(){
    cout << "Turning off the thrusters..." << endl;

    for(int i = 0; i < thrusters.size(); i++) thrusters[i].finishesThruster();

    cout << "Thrusters off" << endl;
}