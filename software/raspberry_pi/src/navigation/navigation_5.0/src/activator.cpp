#include "activator.h"

Activator::Activator(int pin) : pinNumber(pin) {
    if (wiringPiSetup() == -1) throw FailedConnectWiringPi();
    pinMode(pinNumber, INPUT);
}

void Activator::WaitingForActivation() {
    // Esse pino precisa de um resistor de pull-up
    while(digitalRead(pinNumber)) sleep_for(milliseconds(100));
}