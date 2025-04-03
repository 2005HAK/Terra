#include "utils.h"

string getTime(){
    auto now = system_clock::now();
    time_t nowTime = system_clock::to_time_t(now);
    string strNowTime = ctime(&nowTime);
    strNowTime.pop_back();
    return strNowTime;
}