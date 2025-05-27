#include "utils.h"

string getTime(){
    auto now = system_clock::now();
    time_t nowTime = system_clock::to_time_t(now);
    string strNowTime = ctime(&nowTime);
    strNowTime.pop_back();
    return strNowTime;
}

string actionToString(Action action){
    if(Action::FORWARD == action) return "FORWARD";
    else if(Action::BACKWARD == action) return "BACKWARD";
    else if(Action::RIGHT == action) return "RIGHT";
    else if(Action::LEFT == action) return "LEFT";
    else if(Action::UP == action) return "UP";
    else if(Action::DOWN == action) return "DOWN";
    else if(Action::TURNRIGHT == action) return "TURNRIGHT";
    else if(Action::TURNLEFT == action) return "TURNLEFT";
    else if(Action::NONE == action) return "NONE";
    else return "UNKNOWN";
}
