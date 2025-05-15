#include "navigation.h"

string stateToString(State state){
    static unordered_map<State, string> stateNames = {
        {State::INIT, "INIT"},
        {State::SEARCH, "SEARCH"},
        {State::CENTERING, "CENTERING"},
        {State::ADVANCING, "ADVANCING"},
        {State::PASSGATE, "PASSGATE"},
        {State::ALIGNTOPATH, "ALIGNTOPATH"},
        {State::NAVIGATE, "NAVIGATE"},
        {State::DROPMARKERS, "DROPMARKERS"},
        {State::TAGGING, "TAGGING"},
        {State::CLEANUP, "CLEANUP"},
        {State::RETURNING, "RETURNING"},
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

    if(middle[0] >= 0 && middle[0] <= IMAGE_WIDTH && middle[1] >= 0 && middle[1] <= IMAGE_HEIGHT){
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


AUVStateMachine::AUVStateMachine(){
    cout << "Waiting for activation..." << endl;
    Activator act(25); // GPIO 17 = pino 0 no esquema WiringPi
    act.WaitingForActivation();
    cout << "Ativado!\n";

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

// FUNCTIONS USED BY THREADS

void AUVStateMachine::sensorsData(){
    sensors->initialize();

    /*
    while(running){
        sensors->updateData();
	sleep_for(milliseconds(100));
    }
    */
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

void AUVStateMachine::stabilizes(){
    while(running){
        array<Decision, 3> decision;
        array<double, 3> errorVelocity = {.1, .1, .1};
    
        definesAction(decision[0].action, this->sensors->getVel()[0], errorVelocity[0], Action::FORWARD, Action::BACKWARD);
        definesAction(decision[1].action, this->sensors->getVel()[1], errorVelocity[1], Action::RIGHT, Action::LEFT);
        definesAction(decision[2].action, this->sensors->getVel()[2], errorVelocity[2], Action::DOWN, Action::UP);

        if(this->thrusters->getStabilizeVert()){
            this->thrusters->defineAction(decision[0]);
            sleep_for(milliseconds(500));
        }
        if(this->thrusters->getStabilizeHori()){
            this->thrusters->defineAction(decision[1]);
            this->thrusters->defineAction(decision[2]);
            sleep_for(milliseconds(500));
        }
    }
}

// END FUNCTIONS USED BY THREADS

// TRANSITION FUNCTIONS

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

void AUVStateMachine::transitionTo(State newState){
    cout << "Transitioning from " + stateToString(this->state) + " to " + stateToString(newState) << endl;
    this->lastState = this->state;
    this->state = newState;
}

// END TRANSITION FUNCTIONS

// ERRORS HANDLING

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

// END ERRORS HANDLING

// DEFINITION OF STATES

void AUVStateMachine::search(){
    int rotationCurrent = 0;

    cout << "Searching..." << endl;

    // Provavelmente as o modo de encontrar os objetos para fazer as transições sera mudado para ser mais especifico para cada caso

    if(this->lastState == State::INIT){
        this->thrusters->defineAction({Action::NONE, 0});

        while(!searchObjects("Gate")){
            if(rotationCurrent < 1){
                rotate();
                rotationCurrent++;
            } else if(rotationCurrent < 3){
                rotate();
                rotationCurrent++;
            } // tratar questão de não encontrar o gate so com esse procedimento
            sleep_for(milliseconds(100));
        }
    } else if(this->lastState == State::PASSGATE){
        this->yoloCtrl->switchCam(0);
        sleep_for(seconds(2));

        Action action = Action::NONE;

        while(!searchObjects("PathMarker")){
            if(action == Action::NONE){
                if(sideIsLeft) action = Action::TURNRIGHT;
                else action = Action::TURNLEFT;

                rotate(M_PI / 2, 0.174533, action);

                this->thrusters->defineAction({Action::NONE, 0});
                this->thrusters->defineAction({Action::FORWARD, 20});
            }
            this->thrusters->defineAction({Action::FORWARD, 20});
            sleep_for(milliseconds(100));
        }
    } else if(this->lastState == State::ALIGNTOPATH){
        while(!searchObjects("SlalomRed") && !searchObjects("SlalomWhite") && !searchObjects("Bin") && !searchObjects("Torpedoes")){
            this->thrusters->defineAction({Action::FORWARD, 20});
            sleep_for(milliseconds(100));
        }
    } else if(this->lastState == State::NAVIGATE || this->lastState == State::DROPMARKERS){
        this->yoloCtrl->switchCam(0);
        sleep_for(seconds(2));

        int time = 100, count = 0;

        while(!searchObjects("PathMarker")){
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
    } else if (this->lastState == State::TAGGING){
        // Quando tivermos o hidrofone entender os dados e como usa-los para se mover procurando o pinger

        while(!searchObjects("Octagon")){
            // Implementar movimentação para encontrar o pinger baseado no hidrofone
            sleep_for(milliseconds(100));
        }
    }

    this->thrusters->defineAction({Action::NONE, 0});
    checksTransition();
}

void AUVStateMachine::init(){
    cout << "Searching for launcher..." << endl;
    /*
    while(this->targetObject != OBJECT_INITIALIZATION){
        searchObjects();
        sleep_for(milliseconds(100));
    }
    */

    Activator act(6); // GPIO 17 = pino 0 no esquema WiringPi
        act.WaitingForActivation();
        cout << "Ativado!\n";

    cout << "Initializing..." << endl;

    this->thrusters = make_unique<ThrustersControl>();

    if(thrusters){
        stabilizesThread = thread(&AUVStateMachine::stabilizes, this);
        checksTransition();
    }
    else throw FailedConnectThrusters();
}  

void AUVStateMachine::passGate(){
    cout << "Passing gate..." << endl;

    if(centering()){
        while(!searchObjects(INITIALCHOICE)){
            this->thrusters->defineAction({Action::FORWARD, 20});
            sleep_for(milliseconds(100));
        }

        this->thrusters->defineAction({Action::NONE, 0});

        array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);

        // Check which side the object is on
        if(xyxy[0] > IMAGE_CENTER[0]) sideIsLeft = false;

        if(centering()){
            double currentDistance = 0, distance;

            array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);
            calculateDistance(distance, this->targetObject, xyxy);

            bool isAbove = false;
            while(!isAbove){
                array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);
                
                if(xyxy[0] != -1){
                    array<int, 2> centerObject = center(xyxy);
                    
                    if(xyxy[1] > (ERROR_CENTER / 2)) this->thrusters->defineAction({Action::DOWN, 20});
                    else isAbove = true;
                }
            }
            this->thrusters->defineAction({Action::NONE, 0});

            while(currentDistance < distance + .5){
                this->thrusters->defineAction({Action::FORWARD, 20});

                currentDistance += this->sensors->getVel()[0] * this->sensors->deltaTime().count(); // Verificar tamanho da defasagem
            }

            this->thrusters->defineAction({Action::NONE, 0});

            // Rotate after passing through the gate
            rotate(2 * M_PI, 0.174533, Action::TURNRIGHT);
            sleep_for(milliseconds(500));
            rotate(2 * M_PI, 0.174533, Action::TURNRIGHT);
        }
        checksTransition();
    } else{
        // tratar questão de perda de objeto
    }
}

// Testar

void AUVStateMachine::alignToPath(){
    cout << "Aligning to path..." << endl;

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
    }
}

// Testar

void AUVStateMachine::navigate(){
    cout << "Navigating..." << endl;

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
                    decision.action = Action::TURNRIGHT;
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

    checksTransition();
}

// Testar

void AUVStateMachine::dropMarkers(){
    cout << "Dropping markers..." << endl;

    while(searchObjects("Bin")){
        this->thrusters->defineAction({Action::FORWARD, 20});
        sleep_for(milliseconds(100));
    }

    this->thrusters->defineAction({Action::NONE, 0});

    this->yoloCtrl->switchCam(0);
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

    checksTransition();
}

// Implementar

void AUVStateMachine::tagging(){
    cout << "Tagging..." << endl;

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

        /** Terminar o ajuste do AUV e disparo do torpedo, que pode ser feito de 2 maneiras:
         * 1 - Ajustar o AUV com base em testes (mais simples)
         * 2 - Com base na força de disparo, distancia ao alvo, resistencia da agua e local onde o disparador
         *     esta localizado no AUV, calcular onde o AUV dece estar para que o torpedo atinja o alvo (mais 
         *     complexo)
         */

        if(ready) shoot();
        else throw FailedFiringTorpedo();

        checksTransition();
    }
}

// Implementar
void AUVStateMachine::shoot(){
}

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
// Atualizar para que faça a movimentação correta quando estiver usando a camera de baixo
// Atualizar para que identifique se o objeto esta de lado, se tiver as dimensoes dos objetos da para 
// usar proporção

bool AUVStateMachine::centering(){
    cout << "Centering..." << endl;

    bool isCenter = false;

    while(!isCenter){
        array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);

        if(xyxy[0] != -1 && xyxy[1] != -1 && xyxy[2] != -1, xyxy[3] != -1){
            array<Decision, 2> decision;

            centerObject(decision, xyxy);

            this->thrusters->defineAction(decision[0]);
            this->thrusters->defineAction(decision[1]);
            
            if(decision[0].action == Action::NONE && decision[1].action == Action::NONE) isCenter = true;
        } else{
	        isCenter = true;
	        cout << "Lost object!" << endl;
        }
    }

    return isCenter;
}

// Falta definir como fazer isso, ainda não temos a garra que segura os markers

void AUVStateMachine::dropping(){

}

// Testar

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
    if(lostObject){
        /*
        this->nextState = State::STOP;
        transitionTo(State::STABILIZING);
        */
    } else checksTransition();
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

                //verificar se ainda serão usados
                case State::CENTERING:
                    centering();
                    break;
                case State::ADVANCING:
                    advancing();
                    break;
                case State::STABILIZING:
                    stabilizes();
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

// END RUN STATE MACHINE

int main(){
    AUVStateMachine auv;
    auv.run();
    return 0;
}
