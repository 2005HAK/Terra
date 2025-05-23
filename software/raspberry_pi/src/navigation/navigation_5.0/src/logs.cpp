#include "logs.h"

void logMessage(std::string message){
    std::string msg = "[" + getTime() + "] " + message;

    int mode = 2; // 0 - console, 1 - file, 2 - both, 3 - none

    if(mode == 0) std::cout << msg << std::endl;
    else if(mode == 1) logToFile(msg);
    else if(mode == 2){
        std::cout << msg << std::endl;
        logToFile(msg);
    }
}

void logToFile(std::string message){
    std::ofstream logFile("auv.log", std::ios::app);
    if(logFile.is_open()){
        logFile << message << endl;
        logFile.close();
    } else {
        std::cout << "Failed to open log file: auv.log" << std::endl;
    }
}