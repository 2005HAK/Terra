#ifndef ACTIVATOR_H
#define ACTIVATOR_H

#include "utils.h"

class Activator {
    private:
        int pinNumber;

    public:
        /**
         * @brief Constructor for the Activator class.
         * 
         * @param pin The GPIO pin number to be used for activation.
         */
        Activator(int pin);

        /**
         * @brief Waits for activation signal on the specified GPIO pin.
         * 
         * @details This function blocks until the activation signal is received.
         */
        void WaitingForActivation();
};

#endif // ACTIVATOR_H
