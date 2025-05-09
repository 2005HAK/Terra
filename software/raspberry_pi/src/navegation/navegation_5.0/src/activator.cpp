#include "activator.h"
#include <wiringPi.h>
#include <stdexcept>
#include <iostream>

Activator::Activator(int pin) : pinNumber(pin) {
    if (wiringPiSetup() == -1) {
        throw std::runtime_error("Erro ao inicializar WiringPi");
    }
    pinMode(pinNumber, INPUT);
}

void Activator::WaitingForActivation() {
    while (digitalRead(pinNumber)) {
        delay(10);
    }
}
