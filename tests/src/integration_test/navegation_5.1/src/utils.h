#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <chrono>
#include <thread>
#include <cmath.h>

using namespace std;
using namespace chrono;
using namespace this_thread;

/**
 * Actions:
 * 
 * - UP         : Move the AUV up, turning on the thrusters 3 and 6 in the forward direction.
 * - DOWN       : Move the AUV down, turning on the thrusters 3 and 6 in the reverse direction.
 * - FORWARD    : Move the AUV forward, turning on the thrusters 1 and 2 in the reverse direction and the thrusters 4 and 5 in the forward direction.
 * - BACKWARD   : Move the AUV backward, turning on the thrusters 1 and 2 in the forward direction and the thrusters 4 and 5 in the reverse direction.
 * - RIGHT      : Move the AUV right, turning on the thrusters 2 and 4 in the forward direction and the thrusters 1 and 5 in the reverse direction.
 * - LEFT       : Move the AUV left, turning on the thrusters 1 and 5 in the forward direction and the thrusters 2 and 4 in the reverse direction.
 * - TURN RIGHT : Turn the AUV right, turning on the thrusters 2 and 5 in the forward direction and the thrusters 1 and 4 in the reverse direction.
 * - TURN LEFT  : Turn the AUV left, turning on the thrusters 1 and 4 in the forward direction and the thrusters 2 and 5 in the reverse direction.
 * - NONE       : No action.
 */


/**
 * @brief Gets the current time as a string.
 * 
 * @return The current time in string format.
 */
string getTime();

#endif // UTILS_H
