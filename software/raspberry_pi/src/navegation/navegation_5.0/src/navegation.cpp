#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <map>
#include "sensors.cpp"
#include "yoloctrl.cpp"
#include "thrusters_control.cpp"
#include "auv_error.h"

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
const double SAFE_DISTANCE = 1;

// Defines the power maximum that motors can receive (in %) ja existe em thrusters control
// const double POWER_MAX = 25;

enum class State{
    INIT,
    SEARCH,
    CENTERING,
    ADVANCING,
    PASS_GATE_WITH_STYLE,
    STABILIZING,
    STOP,
    NONE
};

string stateToString(State state){
    static unordered_map<State, string> stateNames = {
        {State::INIT, "INIT"},
        {State::SEARCH, "SEARCH"},
        {State::CENTERING, "CENTERING"},
        {State::ADVANCING, "ADVANCING"},
        {State::PASS_GATE_WITH_STYLE, "PASS_GATE_WITH_STYLE"},
        {State::STABILIZING, "STABILIZING"},
        {State::STOP, "STOP"}
    };
    
    auto it = stateNames.find(state);
    return (it != stateNames.end() ? it->second : "UNKNOWN");
}

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
void centerSetPower(array<Decision, 2> &decision, array<int, 2> center){
    double kpH = .5, kpV = .5;

    decision[0].value = max(min(kpH * fabs(center[0] - IMAGE_CENTER[0]), POWER_MAX), .0);
    decision[1].value = max(min(kpV * fabs(center[1] - IMAGE_CENTER[1]), POWER_MAX), .0);
}

void distanceSetPower(double &power, double distance){
    double kpF = 4.5;

    power = max(min(kpF * fabs(distance - SAFE_DISTANCE), POWER_MAX), .0);
}

void velocitySetPower(array<Decision, 3> &decision, array<double, 3> velocity){
    double kpX = 1.5, kpY = 1.5, kpZ = 1.5;

    decision[0].value = max(min(kpX * fabs(velocity[0]), POWER_MAX), .0);
    decision[1].value = max(min(kpY * fabs(velocity[1]), POWER_MAX), .0);
    decision[2].value = max(min(kpZ * fabs(velocity[2]), POWER_MAX), .0);
}

/**
 * @brief Decides which movement to take based on the position of the object in the image.
 * 
 * @param decision Array of struct where the move decision is stored
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

    centerSetPower(decision, middle);
}

/**
 * @brief Calculates the distance berween AUV and object based on the object's actual width and image dimension
 * 
 * @param objectDistance Variable where the distance between the AUV and the object is stored
 * @param objectClass The class of the detected object
 * @param xyxy Coordinates of the bounding box of the detected object
 */
void calculateDistance(double &objectDistance, string objectClass, array<int, 4> xyxy){
    // Actual width of the objects (in meters)
    static map<string, double> widthObjects{{"obj1", 2}, {"Cube", .055}};

    // Inicializes the variable with invalid value to indicates error
    objectDistance = -1;

    auto it = widthObjects.find(objectClass);

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
}

/**
 * @brief Decides whether to advance to the object and the power that will be used
 * 
 * @param decision Struct where the move decision is stored
 * @param objectDistance Distance between the AUV and the object
 */
void advanceDecision(Decision &decision, double objectDistance){
    if(objectDistance > SAFE_DISTANCE){
        decision.action = Action::FORWARD;
        distanceSetPower(decision.value, objectDistance);
    }
}

void definesAction(Action &action, double velocity, double errorVelocity, Action positiveAction, Action negativeAction){
    if(velocity > errorVelocity) action = negativeAction;
    else if(velocity < 0 - errorVelocity) action = positiveAction;
}
/**
 * @brief Defines the actions to stabilize the AUV
 * 
 * @param decision Array of struct where the move decision is stored
 * @param velocity Velocity values on the x, y and z axes, respectively
 */
void stabilizes(array<Decision,3> &decision, array<double, 3> velocity){
    //acceptable error in the velocity
    array<double, 3> errorVelocity = {.1, .1, .1};

    definesAction(decision[0].action, velocity[0], errorVelocity[0], Action::FORWARD, Action::BACKWARD);
    definesAction(decision[1].action, velocity[1], errorVelocity[0], Action::RIGHT, Action::LEFT);
    definesAction(decision[2].action, velocity[2], errorVelocity[0], Action::DOWN, Action::UP);
}

class AUVStateMachine{
    private:
        State lastState = State::NONE;
        State state = State::NONE;
        State nextState = State::NONE;
        string targetObject = "";
        double distance; // passar o calculo e armazenamento de distancia para a yolo
        Sensors *sensors;
        YoloCtrl *yoloCtrl;
        ThrustersControl *thrusters;
        thread sensorThread;
        thread detectionThread;
        thread errorThread;

    public:
        AUVStateMachine(){
            cout << "State machine creation..." << endl;
            this->state = State::INIT;
            this->sensors = new Sensors();
            this->yoloCtrl = new YoloCtrl();
            sleep_for(seconds(5));

            // Update sensors data and detection data in parallel with the state machine
            sensorThread = thread(&AUVStateMachine::sensorsData, this);
            detectionThread = thread(&AUVStateMachine::detectionData, this);
        }

        void sensorsData(){
            sensors->updateData();
        }

        void detectionData(){
            yoloCtrl->updateData();
        }

        /**
         * @brief Checks for errors every **100 ms**
         */
        void checksErrors(){
            while(1){
                sensors->collisionDetect();
                sensors->detectOverheat();
                sleep_for(milliseconds(100));
            }
        }
        /**
         * @brief Transition between states
         * 
         * @param newState Next state of the state machine
         */
        void transitionTo(State newState){
            cout << "Transitioning from " + stateToString(this->state) + "to " + stateToString(newState) << endl;
            this->lastState = this->state;
            this->state = newState;
        }

        //ERRORS HANDLING
        void errorHandling(AUVError e){
            auto* error = dynamic_cast<CollisionDetected*>(&e);
            if(error){
                if(this->state == State::SEARCH) directionCorrection(error->getAcceleration());
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

            rotate(rotationAngle, 0.174533, action);

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
                    Decision decision = {Action::DOWN, 20};
                    this->thrusters->defineAction(decision);
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
            Decision decision = {action, 20};
            
            while(fabs(rotated) < angle - errorAngle){
                this->thrusters->defineAction(decision);

                gyroOld = gyroCurrent;
                gyroCurrent = this->sensors->getGyro();
                // Provalmente ta errado
                double deltaTime = this->sensors->deltaTime().count();

                rotated += deltaTime * (gyroCurrent[2] + gyroOld[2]) / 2;
            }
            decision.value = 0;

            this->thrusters->defineAction(decision);
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

                    this->thrusters->defineAction(decision[0]);
                    this->thrusters->defineAction(decision[1]);
                }
            }

            this->nextState = State::ADVANCING;
            this->transitionTo(State::STABILIZING);
        }

        // Testar
        /**
         * @brief This state difines the advancement procedure
         */
        void advancing(){
            cout << "Advancing..." << endl;

            int lostObject = 0;
            int advance = 1;

            while(advance){
                array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);

                if(xyxy[0] != -1){
                    Decision decision;

                    calculateDistance(this->distance, this->targetObject, xyxy);
                    advanceDecision(decision, this->distance);

                    if(decision.action == Action::NONE) advance == 0;

                    this->thrusters->defineAction(decision);
                } else {
                    advance = 0;
                    lostObject = 1;
                    cout << "Lost object!" << endl;
                }
            }

            if(!lostObject){
                this->nextState = State::STOP;
                transitionTo(State::STABILIZING);
            } else transitionTo(State::SEARCH);
        }

        /**
         * @brief This state stabilizes the AU
         */
        void stabilizing(){
            cout << "Stabilizing..." << endl;

            int isStable = 0;

            while(!isStable){
                array<Decision, 3> decision;
                stabilizes(decision, this->sensors->getVel());

                this->thrusters->defineAction(decision[0]);
                sleep_for(milliseconds(500));
                this->thrusters->defineAction(decision[1]);
                this->thrusters->defineAction(decision[2]);
                sleep_for(milliseconds(500));

                if(decision[0].action == Action::NONE && decision[1].action == Action::NONE && decision[2].action == Action::NONE) isStable = 1;
            }

            transitionTo(this->nextState != State::NONE ? this->nextState : State::SEARCH);
            this->nextState = State::NONE;
        }

        /**
         * @brief This state defines the stopping procedure
         */
        void stop(){
            cout << "Stoping..." << endl;

            this->yoloCtrl->stop();
            this->thrusters->finish();
        }
        // END DEFINITION OF STATES

        /**
         * @brief Initializes the state machine
         */
        void run(){
            try{
                errorThread = thread(&AUVStateMachine::checksErrors, this);

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

        ~AUVStateMachine(){
            if (sensorThread.joinable()) sensorThread.join();
            if (detectionThread.joinable()) detectionThread.join();
            if (errorThread.joinable()) errorThread.join();

            delete this->sensors;
            delete this->thrusters;
            delete this->yoloCtrl;
        }
};

int main(){
    AUVStateMachine auv;
    auv.init();
    return 0;
}
