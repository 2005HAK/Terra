#ifndef LOGS_H
#define LOGS_H

#include <iostream>
#include <fstream>
#include <string>
#include "utils.h"

using namespace std;

/**
 * @brief Logs a message to the console or file based on the mode.
 * 
 * @param message The message to log.
 */
void logMessage(std::string message);

/**
 * @brief Logs a message to a file.
 * 
 * @param message The message to log.
 */
void logToFile(std::string message);

#endif // LOGS_H