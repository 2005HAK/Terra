#include "navegation.h"

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

array<int, 2> center(array<int, 4> xyxy){
    if(xyxy[0] >= 0 && xyxy[0] <= IMAGE_WIDTH && xyxy[1] >= 0 && xyxy[1] <= IMAGE_HEIGHT &&
        xyxy[2] >= 0 && xyxy[2] <= IMAGE_WIDTH && xyxy[3] >= 0 && xyxy[3] <= IMAGE_HEIGHT){
        return {(xyxy[0] + xyxy[2]) / 2, (xyxy[1] + xyxy[3]) / 2};
    }
    return {-1, -1};
}

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

void stabilizes(array<Decision,3> &decision, array<double, 3> velocity){
    //acceptable error in the velocity
    array<double, 3> errorVelocity = {.1, .1, .1};

    definesAction(decision[0].action, velocity[0], errorVelocity[0], Action::FORWARD, Action::BACKWARD);
    definesAction(decision[1].action, velocity[1], errorVelocity[0], Action::RIGHT, Action::LEFT);
    definesAction(decision[2].action, velocity[2], errorVelocity[0], Action::DOWN, Action::UP);
}

AUVStateMachine::AUVStateMachine(){
    cout << "State machine creation..." << endl;
    this->state = State::INIT;
    this->sensors = make_unique<Sensors>();
    this->yoloCtrl = make_unique<YoloCtrl>();
    sleep_for(seconds(5));

    cout << "Threads initialization..." << endl;

    // Update sensors data and detection data in parallel with the state machine
    if(sensors) sensorThread = thread(&AUVStateMachine::sensorsData, this);
    else throw FailedInitializationSensors();

    if(yoloCtrl) detectionThread = thread(&AUVStateMachine::detectionData, this);
    else throw FailedInitializationYolo();
}

AUVStateMachine::~AUVStateMachine(){
    running = false;
    if (sensorThread.joinable()) sensorThread.join();
    if (detectionThread.joinable()) detectionThread.join();
    if (errorThread.joinable()) errorThread.join();
}

void AUVStateMachine::sensorsData(){
    while(running){
        sensors->updateData();
	sleep_for(milliseconds(500));
    }
}

void AUVStateMachine::detectionData(){
    while(running){
        yoloCtrl->updateData();
        sleep_for(milliseconds(100));
    }
}

void AUVStateMachine::checksErrors(){
    while(running){
        if(sensors){
            sensors->collisionDetect();
            sensors->detectOverheat();
        }
        sleep_for(milliseconds(100));
    }
}

void AUVStateMachine::transitionTo(State newState){
    cout << "Transitioning from " + stateToString(this->state) + "to " + stateToString(newState) << endl;
    this->lastState = this->state;
    this->state = newState;
}

//ERRORS HANDLING
void AUVStateMachine::errorHandling(AUVError e){
    auto* error = dynamic_cast<CollisionDetected*>(&e);
    if(error){
        if(this->state == State::SEARCH) directionCorrection(error->getAcceleration());
    }
    
    if(dynamic_cast<const FailedConnectThrusters*>(&e) || dynamic_cast<const HighTemperatureError*>(&e)) exit(1); 
}

void AUVStateMachine::directionCorrection(array<double, 3> acceleration){
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
void AUVStateMachine::init(){
    cout << "Searching for launcher..." << endl;

    while(this->targetObject != OBJECT_INITIALIZATION){
        searchObjects();
    }

    // manter comentado para testes que somente 1 objeto é identificado
    // this->targetObject = "";

    cout << "Initializing..." << endl;

    this->thrusters = make_unique<ThrustersControl>();

    if(thrusters) transitionTo(State::SEARCH);
    else throw FailedConnectThrusters();
}

void AUVStateMachine::search(){
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

void AUVStateMachine::searchObjects(){
    if(this->yoloCtrl->foundObject()) this->targetObject = this->yoloCtrl->greaterConfidanceObject();
}

// Testar
void AUVStateMachine::rotate(double angle, double errorAngle, Action action){
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
void AUVStateMachine::centering(){
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
void AUVStateMachine::advancing(){
    cout << "Advancing..." << endl;

    int lostObject = 0;
    int advance = 1;

    while(advance){
        array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);

        if(xyxy[0] != -1){
            Decision decision;

            calculateDistance(this->distance, this->targetObject, xyxy);
            advanceDecision(decision, this->distance);

            if(decision.action == Action::NONE) advance = 0;

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

void AUVStateMachine::stabilizing(){
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

void AUVStateMachine::stop(){
    cout << "Stoping..." << endl;

    this->yoloCtrl->stop();
    this->thrusters->finish();
}
// END DEFINITION OF STATES

void AUVStateMachine::run(){
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

int main(){
    AUVStateMachine auv;
    auv.run();
    return 0;
}
