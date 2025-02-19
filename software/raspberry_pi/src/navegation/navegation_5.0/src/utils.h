#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;
using namespace this_thread;

/**
 * @brief Actions the AUV can perform
 * 
 * Actions:
 * 
 * - UP         : move the AUV up, turning on the thrusters 3 and 6 in the forward direction.
 * 
 * - DOWN       : move the AUV down, turning on the thrusters 3 and 6 in the reverse direction.
 * 
 * - FORWARD    : move the AUV front, turning on the thrusters 1 and 2 in the reverse direction and the thrusters 4 and 5 in the forward direction.
 * 
 * - BACKWARD   : move the AUV back, turning on the thrusters 1 and 2 in the forward direction and the thrusters 4 and 5 in the reverse direction.
 * 
 * - RIGHT      : move the AUV right, turning on the thrusters 2 and 4 in the forward direction and the thrusters 1 and 5 in the reverse direction.
 * 
 * - LEFT       : move the AUV left, turning on the thrusters 1 and 5 in the forward direction and the thrusters 2 and 4 in the reverse direction.
 * 
 * - TURN RIGHT : turn the AUV right, turning on the thrusters 2 and 5 in the forward direction and the thrusters 1 and 4 in the reverse direction.
 * 
 * - TURN LEFT  : turn the AUV left, turning on the thrusters 1 and 4 in the forward direction and the thrusters 2 and 5 in the reverse direction.
 */
enum class Action{
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT,
    UP,
    DOWN,
    TURNRIGHT,
    TURNLEFT,
    NONE
};

/**
 * Gets the time in string format
 * 
 * @return strNowTime
 */
inline string getTime();

/**
 * @brief Converts the action name to string
 * 
 * @param action Action you want to convert to string
 * 
 * @return Action in string format
 */
inline string actionToString(Action action);

#endif // UTILS_H
