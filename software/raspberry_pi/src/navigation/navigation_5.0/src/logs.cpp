#include "logs.h"

void logMessage(string message){
    string msg = "[" + getTime() + "] " + message;

    int mode = 2; // 0 - console, 1 - file, 2 - both, 3 - none

    if(mode == 0) cout << msg << endl;
    else if(mode == 1) logToFile(msg);
    else if(mode == 2){
        cout << msg << endl;
        logToFile(msg);
    }
}

void logToFile(string message){
    ofstream logFile("auv.log", ios::app);
    if(logFile.is_open()){
        logFile << message << endl;
        logFile.close();
    } else {
        cout << "Failed to open log file: auv.log" << endl;
    }
}