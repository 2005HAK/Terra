#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

using namespace std;
using namespace chrono;
using namespace this_thread;

double currentPower, stablePower = 0, pwmRange = 10000;
double POWER_MAX = 25, FREQUENCY = 400;

int convertValue(int value){
    int minPWMus = 1100, maxPWMus = 1900;

    double usefulPower = (value > 0 ? (100 - stablePower) : (100 + stablePower)) * (value / 100);
    currentPower = max(-POWER_MAX, min(stablePower + usefulPower, POWER_MAX));

    return (FREQUENCY / 1000000) * (minPWMus + ((maxPWMus - minPWMus) / 2) * (1 + (currentPower / 100))) * pwmRange;
}

int main(){
    for(int i = -100; i < 101; i++){
        cout << i << ": " << convertValue(i) << endl;
        sleep_for(milliseconds(100));
    }
    return 0;
}