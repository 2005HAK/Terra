#ifndef NAVEGATION_H
#define NAVEGATION_H

#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <map>
#include "sensors.h"
#include "yoloctrl.h"
#include "thrustersctrl.h"
#include "auverror.h"

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

string stateToString(State state);

/**
 * @brief Calculates the centers of the object
 * 
 * @param xyxy x and y coordinates of the detected object
 * 
 * @return x and y coordinates as a array of center or [-1, -1] if xyxy is invalid
 */
array<int, 2> center(array<int, 4> xyxy);

// Implementação antiga por ainda não foi decidido como fazer o PID
void centerSetPower(array<Decision, 2> &decision, array<int, 2> center);

void distanceSetPower(double &power, double distance);

void velocitySetPower(array<Decision, 3> &decision, array<double, 3> velocity);

/**
 * @brief Decides which movement to take based on the position of the object in the image.
 * 
 * @param decision Array of struct where the move decision is stored
 * @param xyxy x and y coordinates of the detected object
 */
void centerObject(array<Decision, 2> &decision, array<int, 4> xyxy);

/**
 * @brief Calculates the distance berween AUV and object based on the object's actual width and image dimension
 * 
 * @param objectDistance Variable where the distance between the AUV and the object is stored
 * @param objectClass The class of the detected object
 * @param xyxy Coordinates of the bounding box of the detected object
 */
void calculateDistance(double &objectDistance, string objectClass, array<int, 4> xyxy);

/**
 * @brief Decides whether to advance to the object and the power that will be used
 * 
 * @param decision Struct where the move decision is stored
 * @param objectDistance Distance between the AUV and the object
 */
void advanceDecision(Decision &decision, double objectDistance);

void definesAction(Action &action, double velocity, double errorVelocity, Action positiveAction, Action negativeAction);

/**
 * @brief Defines the actions to stabilize the AUV
 * 
 * @param decision Array of struct where the move decision is stored
 * @param velocity Velocity values on the x, y and z axes, respectively
 */
void stabilizes(array<Decision,3> &decision, array<double, 3> velocity);

class AUVStateMachine{
    private:
        State lastState = State::NONE;
        State state = State::NONE;
        State nextState = State::NONE;
        string targetObject = "";
        double distance; // passar o calculo e armazenamento de distancia para a yolo
        unique_ptr<Sensors> sensors;
        unique_ptr<YoloCtrl> yoloCtrl;
        unique_ptr<ThrustersControl> thrusters;
        thread sensorThread;
        thread detectionThread;
        thread errorThread;
        bool running = true;

    public:
        AUVStateMachine();

        ~AUVStateMachine();

        void sensorsData();

        void detectionData();

        /**
         * @brief Checks for errors every **100 ms**
         */
        void checksErrors();

        /**
         * @brief Transition between states
         * 
         * @param newState Next state of the state machine
         */
        void transitionTo(State newState);

        // ERRORS HANDLING

        void errorHandling(AUVError e);

        void directionCorrection(array<double, 3> acceleration);

        // END ERRORS HANDLING

        // DEFINITION OF STATES

        /**
         * @brief This state initializes the thrusters
         */
        void init();

        /**
         * @brief This state defines the search procedure
         */
        void search();

        /**
         * @brief Checks if objects were found. Found saved in target_object
         */
        void searchObjects();

        void rotate(double angle = 0.785398, double errorAngle = 0.174533, Action action = Action::TURNLEFT);

        /**
         * @brief This state defines the centralization procedure
         */
        void centering();

        /**
         * @brief This state difines the advancement procedure
         */
        void advancing();

        /**
         * @brief This state stabilizes the AU
         */
        void stabilizing();

        /**
         * @brief This state defines the stopping procedure
         */
        void stop();
        // END DEFINITION OF STATES

        /**
         * @brief Initializes the state machine
         */
        void run();
};

#endif //NAVEGATION_H
