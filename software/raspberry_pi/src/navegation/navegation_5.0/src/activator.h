#ifndef ACTIVATOR_H
#define ACTIVATOR_H

#include <wiringPi.h>
#include <stdexcept>
#include <iostream>

class Activator {
    private:
        int pinNumber;

    public:
        Activator(int pin);

        void WaitingForActivation();
};

#endif // ACTIVATOR_H
