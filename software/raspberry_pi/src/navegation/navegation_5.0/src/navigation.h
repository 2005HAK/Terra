#ifndef NAVIGATION_H
#define NAVIGATION_H

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

// Initial choice of the AUV
const string INITIALCHOICE = "Reef Shark";

// Center of the image seen by the camera
array<int, 2> IMAGE_CENTER = {IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2};

// Size of the zone that is considered the center of the image (in px)
const int ERROR_CENTER = 50;

// Distance considered safe for the AUV to approach (in m)
const double SAFE_DISTANCE = .1;

/**
 * @brief Enum representing the different states of the AUV state machine.
 */
enum class State{
    INIT,
    SEARCH,
    CENTERING,
    ADVANCING, // VERIFICAR NECESSIDADE
    PASSGATE,
    ALIGNTOPATH,
    NAVIGATE,
    DROPMARKERS,
    TAGGING,
    CLEANUP,
    RETURNING,
    STABILIZING, //VERIFICAR NECESSIDADE
    STOP,
    NONE
};

/**
 * @brief Struct representing a decision to be made by the AUV.
 */
struct StateTransition{
    State lastState;
    State currentState;
    string targetObject;
    State nextState;
}

/**
 * @brief Vector representing the transitions between states.
 */
vector<StateTransition> stateTransitions = {
    {State::NONE, State::INIT, "", State::SEARCH},
    {State::INIT, State::SEARCH, "Gate", State::PASSGATE},
    {State::SEARCH, State::PASSGATE, "", State::SEARCH},
    {State::PASSGATE, State::SEARCH, "PathMarker", State::ALIGNTOPATH},
    {State::SEARCH, State::ALIGNTOPATH, "", State::SEARCH},
    {State::ALIGNTOPATH, State::SEARCH, "SlalomRed", State::NAVIGATE},
    {State::ALIGNTOPATH, State::SEARCH, "SlalomWhite", State::NAVIGATE},
    {State::SEARCH, State::NAVIGATE, "", State::SEARCH},
    {State::NAVIGATE, State::SEARCH, "PathMarker", State::ALIGNTOPATH},
    {State::NAVIGATE, State::ALIGNTOPATH, "", State::SEARCH},
    {State::ALIGNTOPATH, State::SEARCH, "Bin", State::DROPMARKERS},
    {State::SEARCH, State::DROPMARKERS, "", State::SEARCH},
    {State::DROPMARKERS, State::SEARCH, "PathMarker", State::ALIGNTOPATH},
    {State::DROPMARKERS, State::ALIGNTOPATH, "", State::SEARCH},
    {State::ALIGNTOPATH, State::SEARCH, INITIALCHOICE, State::TAGGING},
    {State::SEARCH, State::TAGGING, "", State::SEARCH},
    {State::TAGGING, State::SEARCH, "Octagon", State::CLEANUP},
    {State::SEARCH, State::CLEANUP, "", State::RETURNING},
    {State::CLEANUP, State::RETURNING, "", State::STOP},
}

/**
 * @brief Transforms the state in enum to the state in string.
 * 
 * @param state A state in enum format.
 * @return The state in string format.
 */
string stateToString(State state);

/**
 * @brief Calculates the center of the object.
 * 
 * @param xyxy x and y coordinates of the detected object.
 * @return x and y coordinates as an array of center or [-1, -1] if xyxy is invalid.
 */
array<int, 2> center(array<int, 4> xyxy);

/**
 * @brief Sets the power for centering the object.
 * 
 * @param decision Array of struct where the move decision is stored.
 * @param center Coordinates of the center of the object.
 */
void centerSetPower(array<Decision, 2> &decision, array<int, 2> center);

/**
 * @brief Sets the power based on the distance to the object.
 * 
 * @param power Variable where the power is stored.
 * @param distance Distance to the object.
 */
void distanceSetPower(double &power, double distance);

/**
 * @brief Sets the power based on the velocity of the AUV.
 * 
 * @param decision Array of struct where the move decision is stored.
 * @param velocity Velocity values on the x, y, and z axes, respectively.
 */
void velocitySetPower(array<Decision, 3> &decision, array<double, 3> velocity);

/**
 * @brief Decides which movement to take based on the position of the object in the image.
 * 
 * @param decision Array of struct where the move decision is stored.
 * @param xyxy x and y coordinates of the detected object.
 */
void centerObject(array<Decision, 2> &decision, array<int, 4> xyxy);

/**
 * @brief Calculates the distance between the AUV and the object based on the object's actual width and image dimension.
 * 
 * @param objectDistance Variable where the distance between the AUV and the object is stored.
 * @param objectClass The class of the detected object.
 * @param xyxy Coordinates of the bounding box of the detected object.
 */
void calculateDistance(double &objectDistance, string objectClass, array<int, 4> xyxy);

/**
 * @brief Decides whether to advance to the object and the power that will be used.
 * 
 * @param decision Struct where the move decision is stored.
 * @param objectDistance Distance between the AUV and the object.
 */
void advanceDecision(Decision &decision, double objectDistance);

/**
 * @brief Auxiliary function to the function stabilizes.
 * 
 * @param action Location where the data will be stored.
 * @param velocity Current velocity of the AUV on one axis.
 * @param errorVelocity Acceptable error in the velocity to consider stable.
 * @param positiveAction Action that will be performed if the AUV moves to the negative side of the axis.
 * @param negativeAction Action that will be performed if the AUV moves to the positive side of the axis.
 */
void definesAction(Action &action, double velocity, double errorVelocity, Action positiveAction, Action negativeAction);

/**
 * @brief Defines the actions to stabilize the AUV.
 * 
 * @param decision Array of struct where the move decision is stored.
 * @param velocity Velocity values on the x, y, and z axes, respectively.
 */
void stabilizes(array<Decision,3> &decision, array<double, 3> velocity);

/**
 * @brief Class representing the state machine of the AUV.
 */
class AUVStateMachine{
    private:
        State lastState = State::NONE;
        State state = State::NONE;
        string targetObject = "";
        double distance; // passar o calculo e armazenamento de distancia para a yolo
        unique_ptr<Sensors> sensors;
        unique_ptr<YoloCtrl> yoloCtrl;
        unique_ptr<ThrustersControl> thrusters;
        thread sensorThread;
        thread detectionThread;
        thread errorThread;
        bool sideIsLeft = true;
        bool running = true;

    public:
        AUVStateMachine();

        ~AUVStateMachine();

        /**
         * @brief This function is responsible for calling the function that listens to mavlink messages.
         */
        void sensorsData();

        /**
         * @brief This function is responsible for updating yolo detection data.
         */
        void detectionData();

        /**
         * @brief Checks for errors every 100 ms.
         */
        void checksErrors();

        /**
         * @brief Checks if the state machine should transition to another state.
         */
        bool checksTransition();

        /**
         * @brief Transition between states.
         * 
         * @param newState Next state of the state machine.
         */
        void transitionTo(State newState);

        // ERRORS HANDLING
        
        /**
         * @brief Handles errors that may occur during the execution of the state machine.
         * 
         * @param e Error that occurred.
         */
        void errorHandling(AUVError e);

        /**
         * @brief Corrects the direction of the AUV based on acceleration.
         * 
         * @param acceleration The acceleration values on the x, y, and z axes, respectively.
         */
        void directionCorrection(array<double, 3> acceleration);

        // END ERRORS HANDLING

        // DEFINITION OF STATES
        
        /**
         * @brief This state defines the search procedure.
         */
        void search();

        /**
         * @brief This state initializes the thrusters.
         */
        void init();

        /**
         * @brief This state defines the procedure for passing through the gate.
         */
        void passGate();

        /**
         * @brief This state defines the procedure for aligning the AUV to the path.
         */
        void alignToPath();

        /**
         * @brief This state defines the navigation through the slalom procedure.
         */
        void navigate();

        /**
         * @brief This state defines the procedure for dropping markers.
         */
        void dropMarkers();

        void tagging();

        void cleanup();

        void returning();

        // END DEFINITION OF STATES

        /**
         * @brief Checks if objects were found. Found objects are saved in the targetObject variable.
         */
        bool searchObjects(string object);

        /**
         * @brief Rotates the AUV by a specified angle.
         * 
         * @param angle The angle to rotate (default is 0.785398 radians or 45 degrees).
         * @param errorAngle The acceptable error in the angle (default is 0.174533 radians or 10 degrees).
         * @param action The action to perform (default is Action::TURNLEFT).
         */
        void rotate(double angle = 0.785398, double errorAngle = 0.174533, Action action = Action::TURNLEFT);

        /**
         * @brief This state defines the centralization procedure.
         */
        bool centering();
        
        void dropping();

        /**
         * @brief This state difines the advancement procedure.
         */
        void advancing();

        /**
         * @brief This state stabilizes the AUV.
         */
        void stabilizing();

        /**
         * @brief This state defines the stopping procedure.
         */
        void stop();

        /**
         * @brief Initializes the state machine.
         */
        void run();
};

#endif //NAVIGATION_H
