#include <stdlib.h>
#include <iostream>
#include "sensors.cpp"
#include "yoloctrl.cpp"
#include "thrusters_control.cpp"
#include <string>
#include <thread>
#include <unistd.h>
#include <math.h>

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
}

class AUVStateMachine{
    private:
        State lastState;
        State state;
        State nextState;
        string targetObject = "";
        float distance; // passar o calculo e armazenamento de distancia para a pix
        Sensors sensors;
        YoloCtrl yoloCtrl;
        ThrustersControl thrusters;

    public:
        AUVStateMachine(){
            cout << "State machine creation..." << endl;
            this->state = State::INIT;
            sensors = Sensors();
            yoloCtrl = YoloCtrl();

            // Update sensors data and detection data in parallel with the state machine
            thread sensorThread(sensorsData);
            thread detectionThread(detectionData);
        }

        void sensorsData(){
            sensors.updateData();
        }

        void detectionData(){
            yoloCtrl.updateData();
        }

        void checksErrors(){
            while(1){
                sensors.collisionDetect();
                sensors.detectOverheat();
                sleep(.1);
            }
        }

        void transitionTo(State newState){
            cout << "Transitioning from " + stateToString(this->state) + "to " + stateToString(this->nextState) << endl;
            this->state = newState;
        }

        void run(){
            try{
                thread errorsThread(checksErrors);

                while (this->state != State::STOP){
                    switch (this->state){
                    case State::INIT:
                        init();
                        break;
                    case State::SEARCH:
                        search();
                        break;
                    case State::CENTERING:
                        centering();
                        break;
                    case State::ADVANCING:
                        advancing();
                        break;
                    case State::STABILIZING:
                        stabilizing();
                        break;
                    default:
                        break;
                    }
                }
            } catch(exception e){

            }
        }

        // DEFINITION OF STATES
        void init(){
            cout << "Searching for launcher..." << endl;

            while(this->targetObject != OBJECT_INITIALIZATION){
                searchObjects();
            }

            // manter comentado para testes que somente 1 objeto é identificado
            // this->targetObject = "";

            cout << "Initializing..." << endl;
            this->thrusters = ThrustersControl();
            transitionTo(State::SEARCH);
        }

        void search(){
            int rotationCurrent = 0;

            cout << "Searching..." << endl;

            while(targetObject == ""){
                if(rotationCurrent < 8){
                    rotate();
                    rotationCurrent++;
                } else{
                    this->thrusters.defineAction(Action::DOWN, 20);
                    rotationCurrent = 0;
                }
                searchObjects();
            }
            // verificar qual objeto(os) encontrou e responder de acordo

            transitionTo(State::CENTERING);
        }

        void searchObjects(){
            if(yoloCtrl.foundObject()){
                this->targetObject = yoloCtrl.greaterConfidanceObject();
            }
        }

        void rotate(float angle = 0.785398, float errorAngle = 0.174533, Action action = Action::TURNLEFT){
            float gyroCurrent = sensors.getGyro();
            float gyroOld;
            float rotated = 0;

            while(abs(rotated) < angle - errorAngle){
                thrusters.defineAction(action, 20);

                gyroOld = gyroCurrent;
                gyroCurrent = sensors.getGyro();
                deltaTime
            }
        }
};
