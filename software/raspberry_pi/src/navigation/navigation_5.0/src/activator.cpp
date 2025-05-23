#include "activator.h"

Activator::Activator(int pin) : pinNumber(pin) {
    if (wiringPiSetup() == -1) throw std::runtime_error("Erro ao inicializar WiringPi");
    pinMode(pinNumber, INPUT);
}

void Activator::WaitingForActivation() {
    // Esse pino precisa de um resistor de pull-up
    while(digitalRead(pinNumber)) delay(10);
}
