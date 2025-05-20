#include "utils.h"

string getTime(){
    auto now = std::chrono::system_clock::now();
    time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::string strNowTime = ctime(&nowTime);
    strNowTime.pop_back();
    return strNowTime;
}

string actionToString(Action action){
    if(Action::FORWARD == action) return "FORWARD";
    if else(Action::BACKWARD == action) return "BACKWARD";
    if else(Action::RIGHT == action) return "RIGHT";
    if else(Action::LEFT == action) return "LEFT";
    if else(Action::UP == action) return "UP";
    if else(Action::DOWN == action) return "DOWN";
    if else(Action::TURNRIGHT == action) return "TURNRIGHT";
    if else(Action::TURNLEFT == action) return "TURNLEFT";
    if else(Action::NONE == action) return "NONE";
    else return "UNKNOWN";
}
