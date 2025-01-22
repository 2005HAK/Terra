#include <stdlib.h>
#include <iostream>
//#include "sensors.cpp"
#include "yoloctrl.cpp"
//#include "thrusters_control.cpp"
#include "auv_error.cpp"
#include <string>
#include <thread>
#include <unistd.h>
#include <math.h>
#include <map>

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

// Defines the power maximum that motors can receive (in %)
const int POWER_MAX = 25;

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

struct Decision{
    Action action = Action::NONE;
    int value = -1;
};

/**
 * @brief Calculates the centers of the object
 * 
 * @param xyxy x and y coordinates of the detected object
 * 
 * @return x and y coordinates as a array of center or [-1, -1] if xyxy is invalid
 */
array<int, 2> center(array<int, 4> xyxy){
    if(xyxy[0] >= 0 && xyxy[0] <= IMAGE_WIDTH && xyxy[1] >= 0 && xyxy[1] <= IMAGE_HEIGHT &&
       xyxy[2] >= 0 && xyxy[2] <= IMAGE_WIDTH && xyxy[3] >= 0 && xyxy[3] <= IMAGE_HEIGHT){
        return {(xyxy[0] + xyxy[2]) / 2, (xyxy[1] + xyxy[3]) / 2};
    }
    return {-1, -1};
}

// Implementação antiga por ainda não foi decidido como fazer o PID
array<int, 2> centerSetPower(array<int, 2> center){
    double kpH = .5, kpV = .5;

    int errorH = center[0] - IMAGE_CENTER[0], errorV = center[1] - IMAGE_CENTER[1];

    int powerH = kpH * fabs(errorH), powerV = kpV * fabs(errorV);

    powerH = max(min(powerH, POWER_MAX), 0);
    powerV = max(min(powerV, POWER_MAX), 0);

    return {powerH, powerV};
}

int distanceSetPower(double distance){
    double kpF = 4.5;
    
    int errorF = distance - SAFE_DISTANCE;

    int powerF = kpF = fabs(errorF);

    powerF = max(min(powerF, POWER_MAX), 0);

    return powerF;
}

array<int, 3> velocitySetPower(array<double, 3> velocity){
    double kpX = 1.5, kpY = 1.5, kpZ = 1.5;

    int powerX = kpX * fabs(velocity[0]), powerY = kpY * fabs(velocity[1]), powerZ = kpZ = fabs(velocity[2]);

    powerX = max(min(powerX, POWER_MAX), 0);
    powerY = max(min(powerY, POWER_MAX), 0);
    powerZ = max(min(powerZ, POWER_MAX), 0);

    return {powerX, powerY, powerZ};
}

/**
 * @brief Decides which movement to take based on the position of the object in the image.
 * 
 * @param xyxy x and y coordinates of the detected object
 */
void centerObject(array<Decision, 2> &decision, array<int, 4> xyxy){
    array<int, 2> middle = center(xyxy);

    if(middle[0] >= 0 && middle[0] <= IMAGE_WIDTH && middle[1] >= 0 && middle[1] < IMAGE_HEIGHT){
        if(middle[0] < IMAGE_CENTER[0] - (ERROR_CENTER / 2)) decision[0].action = Action::LEFT;
        else if(middle[0] > IMAGE_CENTER[0] + (ERROR_CENTER / 2)) decision[0].action = Action::RIGHT;

        if(middle[1] < IMAGE_CENTER[1] - (ERROR_CENTER / 2)) decision[1].action = Action::UP;
        else if(middle[1] > IMAGE_CENTER[1] + (ERROR_CENTER / 2)) decision[1].action = Action::DOWN;
    }

    array<int, 2> powers = centerSetPower(middle);

    decision[0].value = powers[0];
    decision[1].value = powers[1];
}

/**
 * @brief Calculates the distance berween AUV and object based on the object's actual width and image dimension
 * 
 * @param objectClass The class of the detected object
 * @param xyxy Coordinates of the bounding box of the detected object
 * 
 * @return The distance between AUV and object (in meters)
 */
double calculateDistance(string objectClass, array<int, 4> xyxy){
    // Actual width of the objects (in meters)
    map<string, double> widthObjects{{"obj1", 2}, {"Cube", .055}};

    // Inicializes the variable with invalid value to indicates error
    double objectDistance = -1;

    map<string, double>::iterator it = widthObjects.find(objectClass);

    if(it != widthObjects.end() && (xyxy[2] - xyxy[0]) != 0){
        // image diagonal (in pixels)
        double d = sqrt(pow(IMAGE_WIDTH, 2) + pow(IMAGE_HEIGHT, 2));

        // Diagonal field of view (in rad)
        double a = (M_PI / 180) * 55;

        // Focal distance
        double f = (d / 2) * (cos(a / 2) / sin(a / 2));

        objectDistance = (f * it->second) / (xyxy[2] - xyxy[0]);
        cout << "Object distance: " << objectDistance << endl;
    }

    return objectDistance;
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

            while(fabs(rotated) < angle - errorAngle){
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

                if(xyxy[0] != -1 && xyxy[1] != -1 && xyxy[2] != -1, xyxy[3] != -1){
                    array<Decision, 2> decision;

                    centerObject(decision, xyxy);

                    this->thrusters->defineAction(decision[0].action, decision[0].value);
                    this->thrusters->defineAction(decision[1].action, decision[1].value);
                }
            }

            this->nextState = State::ADVANCING;
            this->transitionTo(State::STABILIZING);
        }

        void advancing(){

        }

        void stabilizing(){

        }

        void stop(){

        }

        ~AUVStateMachine(){
            delete this->sensors;
            delete this->thrusters;
            delete this->yoloCtrl;
        }
};
