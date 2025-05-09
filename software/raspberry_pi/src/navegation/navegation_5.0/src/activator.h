#ifndef ACTIVATOR_H
#define ACTIVATOR_H

class Activator {
private:
    int pinNumber;

public:
    Activator(int pin);
    void WaitingForActivation();
};

#endif
