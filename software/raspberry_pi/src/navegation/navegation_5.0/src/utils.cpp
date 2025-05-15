#include "utils.h"

string getTime(){
    auto now = system_clock::now();
    time_t nowTime = system_clock::to_time_t(now);
    string strNowTime = ctime(&nowTime);
    strNowTime.pop_back();
    return strNowTime;
}

string actionToString(Action action){
    switch(action){
        case Action::FORWARD: return "FORWARD";
        case Action::BACKWARD: return "BACKWARD";
        case Action::RIGHT: return "RIGHT";
        case Action::LEFT: return "LEFT";
        case Action::UP: return "UP";
        case Action::DOWN: return "DOWN";
        case Action::TURNRIGHT: return "TURNRIGHT";
        case Action::TURNLEFT: return "TURNLEFT";
        case Action::NONE: return "NONE";
        default: return "UNKNOWN";
    }
}
