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
    logMessage("Starting thrusters...");
    
    if(wiringPiSetup() == -1) throw FailedConnectWiringPi();

    initializeThrusters();

    this->defineAction({Action::NONE, 0});

    logMessage("Engines thrusters");
}

void ThrustersControl::initializeThrusters(){
    for(int i = 0; i < PINS.size(); i++){
        if(i == 2 || i == 5) thrusters.emplace_back(PINS[i], -14);
        else thrusters.emplace_back(PINS[i], 0);
    }
    sleep_for(seconds(7));
}

void ThrustersControl::defineAction(Decision decision){
    logMessage("Action: " + actionToString(decision.action) + ", Power: " + to_string(decision.value));
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
            stabilizeHori = false;
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

bool ThrustersControl::getStabilizeVert(){
    return stabilizeVert;
}

bool ThrustersControl::getStabilizeHori(){
    return stabilizeHori;
}

void ThrustersControl::finish(){
    logMessage("Turning off the thrusters...");

    for(int i = 0; i < thrusters.size(); i++) thrusters[i].finishesThruster();

    logMessage("Thrusters off");
}