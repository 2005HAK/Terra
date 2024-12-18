#include <stdlib.h>
#include <iostream>
#include "sensors.cpp"
#include "yoloctrl.cpp"
#include "thrusters.cpp"
#include <string>

using namespace std;

// Width and height of the image seen by the camera
const int IMAGE_WIDTH = 640;
const int IMAGE_HEIGHT = 480;

// Object used to initialization
const string OBJECT_INITIALIZATION = "Cube";

// Size of the zone that is considered the center of the image
const int ERROR_CENTER = 50;

// Distance considered safe for the AUV to approach
const float SAFE_DISTANCE = 1;

enum class State{
    INIT,
    SEARCH,
    CENTERING,
    ADVANCING,
    PASS_GATE_WITH_STYLE,
    STABILIZING,
    STOP
};

string stateToString(State state){
    switch (state){
        case State::INIT:
            return "INIT";
        case State::SEARCH:
            return "SEARCH";
        case State::CENTERING:
            return "CENTERING";
        case State::ADVANCING:
            return "ADVANCING";
        case State::PASS_GATE_WITH_STYLE:
            return "PASS_GATE_WITH_STYLE";
        case State::STABILIZING:
            return "STABILIZING";
        case State::STOP:
            return "STOP";
        default:
            return "UNKNOWN";
}

class AUVStateMachine{
    private:
        State lastState;
        State state;
        State nextState;
        string targetObject = "";
        float distance;
        Sensors sensors;
        YoloCtrl yoloCtrl;
        Thrusters thrusters;
    public:
        AUVStateMachine(){
            cout << "State machine creation..." << endl;
            
        }

        void transitionTo(State newState){
            cout << "Transitioning from " + stateToString(this->state) + "to " + stateToString(this->nextState) << endl;
            this->state = newState;
        }
};
