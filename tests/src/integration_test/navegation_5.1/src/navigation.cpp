#include "navigation.h"

string stateToString(State state){
    static unordered_map<State, string> stateNames = {
        {State::INIT, "INIT"},
        {State::SEARCH, "SEARCH"},
        {State::PASSGATE, "PASSGATE"},
        {State::ALIGNTOPATH, "ALIGNTOPATH"},
        {State::NAVIGATE, "NAVIGATE"},
        {State::DROPMARKERS, "DROPMARKERS"},
        {State::TAGGING, "TAGGING"},
        {State::CLEANUP, "CLEANUP"},
        {State::RETURNING, "RETURNING"},
        {State::STOP, "STOP"}
    };
    
    auto it = stateNames.find(state);
    return (it != stateNames.end() ? it->second : "UNKNOWN");
}

//logica não precisou ser mudada
array<int, 2> center(array<int, 4> xyxy){
    if(xyxy[0] >= 0 && xyxy[0] <= IMAGE_WIDTH && xyxy[1] >= 0 && xyxy[1] <= IMAGE_HEIGHT &&
        xyxy[2] >= 0 && xyxy[2] <= IMAGE_WIDTH && xyxy[3] >= 0 && xyxy[3] <= IMAGE_HEIGHT){
        return {(xyxy[0] + xyxy[2]) / 2, (xyxy[1] + xyxy[3]) / 2};
    }
    return {-1, -1};
}

// ja segue a nova logica de movimentação
void centerObject(array<double, 2> &decision, array<int, 4> xyxy){
    array<int, 2> middle = center(xyxy);

    if(middle[0] >= 0 && middle[0] <= IMAGE_WIDTH && middle[1] >= 0 && middle[1] <= IMAGE_HEIGHT){
        if(middle[0] < IMAGE_CENTER[0] - (ERROR_CENTER / 2)) decision[0] = (1 / (IMAGE_CENTER[0] / 2)) * (middle[0] - IMAGE_CENTER[0]);
        else if(middle[0] > IMAGE_CENTER[0] + (ERROR_CENTER / 2)) decision[0] = (1 / (IMAGE_CENTER[0] / 2)) * (middle[0] - IMAGE_CENTER[0]);

        if(middle[1] < IMAGE_CENTER[1] - (ERROR_CENTER / 2)) decision[1] = (1 / (IMAGE_CENTER[1] / 2)) * (middle[1] - IMAGE_CENTER[1]);
        else if(middle[1] > IMAGE_CENTER[1] + (ERROR_CENTER / 2)) decision[1] = (1 / (IMAGE_CENTER[1] / 2)) * (middle[1] - IMAGE_CENTER[1]);
    }
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

AUVStateMachine::AUVStateMachine(){
    cout << "State machine creation..." << endl;
    this->state = State::INIT;
    this->sensors = make_unique<Sensors>();
    this->yoloCtrl = make_unique<YoloCtrl>();

    cout << "Threads initialization..." << endl;

    // Update sensors data and detection data in parallel with the state machine
    if(sensors) sensorThread = thread(&AUVStateMachine::sensorsData, this);
    else throw FailedInitializationSensors();

    if(yoloCtrl) detectionThread = thread(&AUVStateMachine::detectionData, this);
    else throw FailedInitializationYolo();

    cout << "Threads initialized" << endl;
    cout << "State Machine created" << endl;
}

AUVStateMachine::~AUVStateMachine(){
    running = false;
    if (sensorThread.joinable()) sensorThread.join();
    if (detectionThread.joinable()) detectionThread.join();
    if (errorThread.joinable()) errorThread.join();
}

// ja segue a nova logica de movimentação
// FUNCTIONS USED BY THREADS

void AUVStateMachine::sensorsData(){
    sensors->initialize();

    while(running){
        sensors->updateData();
        sleep_for(milliseconds(100));
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

// END FUNCTIONS USED BY THREADS

// TRANSITION FUNCTIONS
// não precisou mudar a logica
bool AUVStateMachine::checksTransition(){
    if(this->yoloCtrl->foundObject()) {
        for(const auto& transition : stateTransitions){
            if(transition.lastState == this->lastState && transition.currentState == this->state){
                array<int, 4> xyxy = this->yoloCtrl->getXYXY(transition.targetObject);
                if(xyxy[0] != -1 || transition.nextState == State::SEARCH){
                    this->targetObject = transition.targetObject;
                    transitionTo(transition.nextState);
                    return true;
                }               
            }
        }
    }       
    return false;
}
/// não precisou mudar a logica
void AUVStateMachine::transitionTo(State newState){
    cout << "Transitioning from " + stateToString(this->state) + " to " + stateToString(newState) << endl;
    this->lastState = this->state;
    this->state = newState;
}

// END TRANSITION FUNCTIONS

// ERRORS HANDLING
// arrumar logica por conta da movimentação que foi mudada
void AUVStateMachine::errorHandling(AUVError e){
   /* auto* error = dynamic_cast<CollisionDetected*>(&e);
    if(error){
        if(this->state == State::SEARCH) directionCorrection(error->getAcceleration());
    }
    
    if(dynamic_cast<const FailedConnectThrusters*>(&e) || dynamic_cast<const HighTemperatureError*>(&e)) exit(1); */
}

// arrumar logica por conta da movimentação que foi mudada
void AUVStateMachine::directionCorrection(array<double, 3> acceleration){
/*    cout << "Correcting direction..." << endl;

    array<double, 3> positionCollision = {-acceleration[0], -acceleration[1], -acceleration[2]};

    double angle = (acos(positionCollision[0] / sqrt(pow(positionCollision[0], 2) + pow(positionCollision[1], 2))) * M_PI) / 180;
    double rotationAngle = M_PI - angle;

    Action action = Action::TURNRIGHT;

    if(positionCollision[1] > 0) action = Action::TURNLEFT;

    rotate(rotationAngle, 0.174533, action);

    run();*/
}

// END ERRORS HANDLING

// DEFINITION OF STATES
// arrumar logica por conta da movimentação que foi mudada
void AUVStateMachine::search(){
    int rotationCurrent = 0;

    cout << "Searching..." << endl;

    // Provavelmente as o modo de encontrar os objetos para fazer as transições sera mudado para ser mais especifico para cada caso

    if(this->lastState == State::INIT){
        while(!searchObjects("Cube")){
            double rotationAngleInit = this->sensors->getPosition()[4];
            this->thrusters->rotateYaw(.785398); // 45 degrees
            while(this->sensors->getPosition()[4] - rotationAngleInit < .785398 - ERROR_ANGLE){
                sleep_for(milliseconds(100));
            }
        }
    } else if(this->lastState == State::PASSGATE){
        //this->yoloCtrl->switchCam(1);
        //sleep_for(seconds(2));

        for(int i = 0; i < 4; i++){
            calculateDistance(this->distance, "Cube", this->yoloCtrl->getXYXY("Cube"));
            sleep_for(milliseconds(100));
        }
    } else transitionTo(State::STOP);

    checksTransition();
}

// ja segue a nova logica de movimentação
void AUVStateMachine::init(){
    cout << "Searching for launcher..." << endl;

    // implementar logica do sensor de agua
    while(!searchObjects(OBJECT_INITIALIZATION)){
        sleep_for(milliseconds(100));
    }
    
    cout << "Initializing..." << endl;

    this->thrusters = make_unique<ThrustersControl>(this->sensors.get()); //ERRO AQUI

    if(thrusters){
        this->thrusters->moveZ(.5);
        checksTransition();
    }
    else throw FailedConnectThrusters();
}  

// ja segue a nova logica de movimentação TESTAR
void AUVStateMachine::passGate(){
    cout << "Passing gate..." << endl;

    if(centering()){
        while(!searchObjects(INITIALCHOICE)){
            double positionInit = this->sensors->getPosition()[0], distanceMove = .1;
            this->thrusters->moveX(distanceMove);

            while(this->sensors->getPosition()[0] - positionInit < distanceMove - ERROR_DISTANCE) sleep_for(milliseconds(100));
        }

        // Rotate after passing through the gate
        this->thrusters->rotateYaw(2 * M_PI);
        sleep_for(milliseconds(500));
        this->thrusters->rotateYaw(2 * M_PI);

        checksTransition();
    } else{
        this->state = State::STOP;
    }

    cout << "Gate passed!" << endl;
}

// Testar

void AUVStateMachine::alignToPath(){
    /*cout << "Aligning to path..." << endl;

    if(centering()){
        array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);
        bool isAlign = false;

        double currentDiference = xyxy[2] - xyxy[0], lastDiference = currentDiference;       
        Decision decision = {Action::TURNRIGHT, 20};
        int switchs = 0;
        
        while(!isAlign){
            
            if(currentDiference > lastDiference){
                if(decision.action == Action::TURNRIGHT) decision.action = Action::TURNLEFT;
                else decision.action = Action::TURNRIGHT;
                switchs++;
            }

            this->thrusters->defineAction(decision);

            lastDiference = currentDiference;
            xyxy = this->yoloCtrl->getXYXY(this->targetObject);
            currentDiference = xyxy[2] - xyxy[0];

            if(switchs > 5) isAlign = true;
        }
        this->yoloCtrl->switchCam(0);
        sleep_for(seconds(2));

        checksTransition();
    }*/
}

// Testar

void AUVStateMachine::navigate(){
    /*cout << "Navigating..." << endl;

    int safeRange = 100;
    int count = 0;
    bool finished = false;

    while(!finished){
        array<int, 4> pipeWhite = this->yoloCtrl->getXYXY("SlalomWhite");
        array<int, 4> pipeRed = this->yoloCtrl->getXYXY("SlalomRed");

        Decision decision = {Action::NONE, 0};
        
        if(pipeWhite[0] != -1 || pipeRed[0] != -1){
            if(pipeWhite[0] != -1 && pipeRed[0] != -1){
                int middlePipes = pipeWhite[2] + ((pipeRed[0] - pipeWhite[2]) / 2);

                if(middlePipes < IMAGE_CENTER[0] - (ERROR_CENTER / 2)){
                    decision.action = Action::TURNLEFT;
                    //Definir com o controle PID
                    decision.value = 20; 
                } else if(middlePipes > IMAGE_CENTER[0] + (ERROR_CENTER / 2)){
                    desision.action = Action::TURNRIGHT;
                    //Definir com o controle PID
                    decision.value = 20; 
                } else {
                    decision.action = Action::FORWARD;
                    //Definir com o controle PID
                    decision.value = 20;
                }
            }else if(pipeWhite[2] > IMAGE_CENTER[0] - safeRange){
                decision.action = Action::TURNRIGHT;
                //Definir com o controle PID
                decision.value = 20; 
            } else if(pipeRed[2] < IMAGE_CENTER[0] + safeRange){
                decision.action = Action::TURNLEFT;
                //Definir com o controle PID
                decision.value = 20; 
            } else{
                decision.action = Action::FORWARD;
                //Definir com o controle PID
                decision.value = 20;
            }
        } else count++;

        if(decision.action != Action::NONE){
            count = 0;
            this->thrusters->defineAction(decision);
            sleep_for(milliseconds(100));
        } else this->thrusters->defineAction({Action::FORWARD, 20});

        if(count > 5) finished = true;

        sleep_for(milliseconds(100));
    }
    // Tratar caso de não haver o pathmarker abaixo do auv neste momento
    this->thrusters->defineAction({Action::NONE, 0});

    checksTransition();*/
}

// Testar

void AUVStateMachine::dropMarkers(){
    /*cout << "Dropping markers..." << endl;

    while(searchObjects("Bin")){
        this->thrusters->defineAction({Action::FORWARD, 20});
        sleep_for(milliseconds(100));
    }

    this->thrusters->defineAction({Action::NONE, 0});

    this->yoloCtrl->switchCam(1);
    sleep_for(seconds(2));

    // Provavelmete terá uma diferenca de profundidade minima para dropar os marcadores, 
    // tratar isso quando tiver mais detalhes
    
    int count = 0, time = 100;
    while(!searchObjects("Bin")){
        rotate(M_PI / 2, 0.174533, Action::TURNRIGHT);
        this->thrusters->defineAction({Action::FORWARD, 20});
        sleep_for(milliseconds(time));
        count++;

        // Revisar esse procedimento de procura pelo pathMarker, não é o suficiente

        if(count == 4){
            time += 100;
            count = 0;
        }
    }

    this->thrusters->defineAction({Action::NONE, 0});

    centering();

    // Com base em testes avançar um pouco, ja que a camera fica mais a frente do que o mecanismo que segura 
    // os marcadores

    // Estabilizar continuamente em um thread separado
    dropping();

    checksTransition();*/
}

// Implementar

void AUVStateMachine::tagging(){
    /*cout << "Tagging..." << endl;

    // In meters
    double minDistance = 1;

    if(centering()){
        double currentDistance, tolerance = .05;
        bool isOk = false;
        // Essa função não precisa do terceiro argumento
        calculateDistance(currentDistance, this->targetObject, this->yoloCtrl->getXYXY(this->targetObject));

        while(!isOk){
            if(currentDistance > minDistance + tolerance){
                this->thrusters->defineAction({Action::FORWARD, 20});
                sleep_for(milliseconds(100));
            } else if(currentDistance < minDistance - tolerance){
                this->thrusters->defineAction({Action::BACKWARD, 20});
                sleep_for(milliseconds(100));
            } else isOk = true;

            calculateDistance(currentDistance, this->targetObject, this->yoloCtrl->getXYXY(this->targetObject));
        }

        bool ready = false;

        if(ready) shoot();
        else throw FailedFiringTorpedo();

        checksTransition();
    }*/
}

// Implementar

void AUVStateMachine::cleanup(){

}

// Implementar

void AUVStateMachine::returning(){

}

//END DEFINITION OF STATES

bool AUVStateMachine::searchObjects(string object){
    if(this->yoloCtrl->foundObject()){
        if(this->yoloCtrl->getXYXY(object)[0] != -1){
            this->targetObject = object;
            return true;
        }
    } 
    return false;
}

// Testar
// Atualizar para que faça a movimentação correta quando estiver usando a camera de baixo
// Atualizar para que identifique se o objeto esta de lado, se tiver as dimensoes dos objetos da para 
// usar proporção

bool AUVStateMachine::centering(){
    cout << "Centering..." << endl;

    bool isCenter = false, lostObject = false;

    while(!isCenter && !lostObject){
        array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);

        if(xyxy[0] != -1){
            array<double, 2> decision = {0, 0};

            centerObject(decision, xyxy);

            this->thrusters->moveY(decision[0]);
            this->thrusters->moveZ(decision[1]);

            if(decision[0] == 0 && decision[1] == 0) isCenter = true;
        } else{
            lostObject = true;
            cout << "Lost object!" << endl;
        }
    }
    // esse retorno é problematico, pois o AUV pode ter perdido o objeto
    return isCenter;
}

// Falta definir como fazer isso, ainda não temos a garra que segura os markers

void AUVStateMachine::dropping(){

}

void AUVStateMachine::stop(){
    cout << "Stoping..." << endl;

    this->yoloCtrl->stop();
    this->thrusters->finish();
}

// RUN STATE MACHINE

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
                case State::PASSGATE:
                    passGate();
                    break;
                case State::ALIGNTOPATH:
                    alignToPath();
                    break;
                case State::NAVIGATE:
                    navigate();
                    break;
                case State::DROPMARKERS:
                    dropMarkers();
                    break;
                case State::TAGGING:
                    //tagging();
                    break;
            }
        }

        stop();
    } catch(AUVError e){
        errorHandling(e);
    }
}

// END RUN STATE MACHINE

int main(){
    AUVStateMachine auv;
    auv.run();
    return 0;
}
