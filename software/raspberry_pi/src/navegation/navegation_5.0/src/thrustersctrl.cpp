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

ThrustersControl::ThrustersControl(){
    cout << "Starting thrusters..." << endl;
    
    if(wiringPiSetup() == -1) throw FailedConnectThrusters();

    initializeThrusters();

    cout << "Engines thrusters" << endl;
}

void ThrustersControl::initializeThrusters(){
    for(int i = 0; i < PINS.size(); i++){
        if(PINS[i] == 27 || PINS[i] == 22) thrusters.emplace_back(PINS[i], -14);
        else thrusters.emplace_back(PINS[i], 0);
    }
    sleep_for(seconds(7));
}

void ThrustersControl::defineAction(Decision decision){
    cout << "Action: " << actionToString(decision.action) << ", Power: " << decision.value << endl;

    switch (decision.action){
        case Action::UP:
            thrusters[2].move(decision.value);
            thrusters[5].move(decision.value);
            break;
        case Action::DOWN:
            thrusters[2].move(-decision.value);
            thrusters[5].move(-decision.value);
            break;
        case Action::FORWARD:
            thrusters[0].move(-decision.value);
            thrusters[1].move(-decision.value);
            thrusters[3].move(decision.value);
            thrusters[4].move(decision.value);
            break;
        case Action::BACKWARD:
            thrusters[0].move(decision.value);
            thrusters[1].move(decision.value);
            thrusters[3].move(-decision.value);
            thrusters[4].move(-decision.value);
            break;
        case Action::RIGHT:
            thrusters[0].move(-decision.value);
            thrusters[1].move(decision.value);
            thrusters[3].move(decision.value);
            thrusters[4].move(-decision.value);
            break;
        case Action::LEFT:
            thrusters[0].move(decision.value);
            thrusters[1].move(-decision.value);
            thrusters[3].move(-decision.value);
            thrusters[4].move(decision.value);
            break;
        case Action::TURNRIGHT:
            thrusters[0].move(-decision.value);
            thrusters[1].move(decision.value);
            thrusters[3].move(-decision.value);
            thrusters[4].move(decision.value);
            break;
        case Action::TURNLEFT:
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

void ThrustersControl::finish(){
    cout << "Turning off the thrusters..." << endl;

    for(int i = 0; i < thrusters.size(); i++) thrusters[i].finishesThruster();

    cout << "Thrusters off" << endl;
}
