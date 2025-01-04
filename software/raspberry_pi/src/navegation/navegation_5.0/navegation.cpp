#include <stdlib.h>
#include <iostream>
#include "sensors.cpp"
#include "yoloctrl.cpp"
#include "thrusters_control.cpp"
#include "auv_error.cpp"
#include <string>
#include <thread>
#include <unistd.h>
#include <math.h>

using namespace std;
using namespace this_thread;

// Width and height of the image seen by the camera
const int IMAGE_WIDTH = 640;
const int IMAGE_HEIGHT = 480;

// Object used to initialization
const string OBJECT_INITIALIZATION = "Cube";

// Center of the image seen by the camera
array<int, 2> IMAGE_CENTER = {IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2};

// Size of the zone that is considered the center of the image (in px)
const int ERROR_CENTER = 50;

// Distance considered safe for the AUV to approach (in m)
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

struct decision{
    Action action;
    int value;
};

/**
 * @brief Calculates the centers of the object
 * 
 * @param xyxy x and y coordinates of the detected object
 * 
 * @return x and y coordinates as a array of center or [-1, -1] if xyxy is null
 */
array<int, 2> center(array<int, 4> xyxy){
    array<int, 2> middle = {-1, -1};

    if(xyxy != NULL){
        middle[0] = (xyxy[0] + xyxy[2]) / 2;
        middle[1] = (xyxy[1] + xyxy[3]) / 2;
    }

    return middle;
}

array<decision, 2> centerObject(array<int, 4> xyxy){
    Action dirH, dirV;
    int powerH, powerV;
    array<int, 2> middle = center(xyxy);

    if(middle[0] >= 0 && middle[0] <= IMAGE_WIDTH && middle[1] >= 0 && middle[1] < IMAGE_HEIGHT){
        if(middle[0] < IMAGE_CENTER[0] - (ERROR_CENTER / 2)) dirH = Action::LEFT;
        else if(middle[0] > IMAGE_CENTER[0] + (ERROR_CENTER / 2)) dirH = Action::RIGHT;

        if(middle[1] < IMAGE_CENTER[1] - (ERROR_CENTER / 2)) dirH = Action::UP;
        else if(middle[1] > IMAGE_CENTER[1] + (ERROR_CENTER / 2)) dirH = Action::DOWN;
    }

    //terminar
}

class AUVStateMachine{
    private:
        State lastState;
        State state;
        State nextState;
        string targetObject = "";
        float distance; // passar o calculo e armazenamento de distancia para a pix
        Sensors *sensors;
        YoloCtrl *yoloCtrl;
        ThrustersControl *thrusters;

    public:
        AUVStateMachine(){
            cout << "State machine creation..." << endl;
            this->state = State::INIT;
            this->sensors = new Sensors();
            this->yoloCtrl = new YoloCtrl();

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

        /**
         * @brief Checks for errors every **100 ms**
         */
        void checksErrors(){
            while(1){
                sensors.collisionDetect();
                sensors.detectOverheat();
                sleep_for(milliseconds(100));
            }
        }
        /**
         * @brief Transition between states
         * 
         * @param newState Next state of the state machine
         */
        void transitionTo(State newState){
            cout << "Transitioning from " + stateToString(this->state) + "to " + stateToString(this->nextState) << endl;
            this->lastState = this->state;
            this->state = newState;
        }

        /**
         * @brief Initializes the state machine
         */
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

                stop();
            } catch(AUVError e){
                errorHandling(e);
            }
        }

        //ERRORS HANDLING
        void errorHandling(AUVError e){
            if(dynamic_cast<const CollisionDetected*>(&e)){
                if(this->state == State::SEARCH) directionCorrection(e.acceleration);
            }
            if(dynamic_cast<const FailedConnectThrusters*>(&e) || dynamic_cast<const HighTemperatureError*>(&e)) exit(1); 
        }

        void directionCorrection(array<double, 3> acceleration){
            cout << "Correcting direction..." << endl;

            array<double, 3> positionCollision = {-acceleration[0], -acceleration[1], -acceleration[2]};

            double angle = (acos(positionCollision[0] / sqrt(pow(positionCollision[0], 2) + pow(positionCollision[1], 2))) * M_PI) / 180;
            double rotationAngle = M_PI - angle;

            Action action = Action::TURNRIGHT;

            if(positionCollision[1] > 0) action = Action::TURNLEFT;

            rotate(angle=rotationAngle, action=action);

            run();
        }

        // DEFINITION OF STATES

        /**
         * @brief This state initializes the thrusters
         */
        void init(){
            cout << "Searching for launcher..." << endl;

            while(this->targetObject != OBJECT_INITIALIZATION){
                searchObjects();
            }

            // manter comentado para testes que somente 1 objeto é identificado
            // this->targetObject = "";

            cout << "Initializing..." << endl;

            this->thrusters = new ThrustersControl();
            transitionTo(State::SEARCH);
        }

        /**
         * @brief This state defines the search procedure
         */
        void search(){
            // 1/8 turns
            int rotationCurrent = 0;

            cout << "Searching..." << endl;

            while(targetObject == ""){
                if(rotationCurrent < 8){
                    rotate();
                    rotationCurrent++;
                } else{
                    // terminar essa parte
                    this->thrusters.defineAction(Action::DOWN, 20);
                    rotationCurrent = 0;
                }

                searchObjects();
            }
            // verificar qual objeto(os) encontrou e responder de acordo

            transitionTo(State::CENTERING);
        }

        /**
         * @brief Checks if objects were found. Found saved in target_object
         */
        void searchObjects(){
            if(this->yoloCtrl->foundObject()) this->targetObject = this->yoloCtrl->greaterConfidanceObject();
        }

        // Testar
        void rotate(double angle = 0.785398, double errorAngle = 0.174533, Action action = Action::TURNLEFT){
            array<double, 3> gyroCurrent = this->sensors->getGyro(), gyroOld;
            double rotated = 0;

            while(abs(rotated) < angle - errorAngle){
                this->thrusters->defineAction(action, 20);

                gyroOld = gyroCurrent;
                gyroCurrent = this->sensors->getGyro();
                // Provalmente ta errado
                steady_clock::time_point deltaTime = this->sensors->deltaTime();

                rotated += deltaTime * (gyroCurrent[2] + gyroOld[2]) / 2;
            }

            this->thrusters->defineAction(action, 0);
        }

        // Testar
        /**
         * @brief This state defines the centralization procedure
         */
        void centering(){
            cout << "Centering..." << endl;

            int lostObject = 0, isCenter = 0;

            while(!isCenter){
                array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);

                if(xyxy != NULL){
                    //terminar

                    this->thrusters->defineAction()
                } else{

                }
            }
        }

        ~AUVStateMachine(){
            delete this->sensors;
            delete this->thrusters;
            delete this->yoloCtrl;
        }
};
