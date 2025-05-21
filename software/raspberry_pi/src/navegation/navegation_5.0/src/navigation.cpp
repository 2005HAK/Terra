#include "navigation.h"

std::string stateToString(State state){
    static unordered_map<State, std::string> stateNames = {
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

std::array<int, 2> center(std::array<int, 4> xyxy){
    if(xyxy[0] >= 0 && xyxy[0] <= IMAGE_WIDTH && xyxy[1] >= 0 && xyxy[1] <= IMAGE_HEIGHT &&
        xyxy[2] >= 0 && xyxy[2] <= IMAGE_WIDTH && xyxy[3] >= 0 && xyxy[3] <= IMAGE_HEIGHT){
        return {(xyxy[0] + xyxy[2]) / 2, (xyxy[1] + xyxy[3]) / 2};
    }
    return {-1, -1};
}

void centerSetPower(std::array<Decision, 2> &decision, std::array<int, 2> center){
    double kpH = .5, kpV = .5;

    decision[0].value = max(min(kpH * fabs(center[0] - IMAGE_CENTER[0]), POWER_MAX), .0);
    decision[1].value = max(min(kpV * fabs(center[1] - IMAGE_CENTER[1]), POWER_MAX), .0);
}

void distanceSetPower(int &power, double distance){
    double kpF = 4.5;

    power = max(min(kpF * fabs(distance - SAFE_DISTANCE), POWER_MAX), .0);
}

void velocitySetPower(std::array<Decision, 3> &decision, std::array<double, 3> velocity){
    double kpX = 1.5, kpY = 1.5, kpZ = 1.5;

    decision[0].value = max(min(kpX * fabs(velocity[0]), POWER_MAX), .0);
    decision[1].value = max(min(kpY * fabs(velocity[1]), POWER_MAX), .0);
    decision[2].value = max(min(kpZ * fabs(velocity[2]), POWER_MAX), .0);
}

void centerObject(std::array<Decision, 2> &decision, std::array<int, 4> xyxy){
    std::array<int, 2> middle = center(xyxy);

    if(middle[0] >= 0 && middle[0] <= IMAGE_WIDTH && middle[1] >= 0 && middle[1] <= IMAGE_HEIGHT){
        if(middle[0] < IMAGE_CENTER[0] - (ERROR_CENTER / 2)) decision[0].action = Action::LEFT;
        else if(middle[0] > IMAGE_CENTER[0] + (ERROR_CENTER / 2)) decision[0].action = Action::RIGHT;

        if(middle[1] < IMAGE_CENTER[1] - (ERROR_CENTER / 2)) decision[1].action = Action::UP;
        else if(middle[1] > IMAGE_CENTER[1] + (ERROR_CENTER / 2)) decision[1].action = Action::DOWN;
    }

    centerSetPower(decision, middle);
}

//Essa função deveria esta em yolocrtl
// Tratar questão do objeto estar de lado

void calculateDistance(double &objectDistance, std::string objectClass, std::array<int, 4> xyxy){
    static map<std::string, double> widthObjects{{"obj1", 2}, {"Cube", .055}}; // Actual width of the objects (in meters)
    auto it = widthObjects.find(objectClass);
    objectDistance = -1;                                                  // Inicializes the variable with invalid value to indicates error

    if(it != widthObjects.end() && (xyxy[2] - xyxy[0]) != 0){
        double d = sqrt(pow(IMAGE_WIDTH, 2) + pow(IMAGE_HEIGHT, 2));      // image diagonal (in pixels)
        double a = (M_PI / 180) * 55;                                     // Diagonal field of view (in rad)
        double f = (d / 2) * (cos(a / 2) / sin(a / 2));                   // Focal distance

        objectDistance = (f * it->second) / (xyxy[2] - xyxy[0]);
        logMessage("Object distance: " + to_string(objectDistance));
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
    logMessage("Waiting for activation...");
    Activator act(6); // GPIO 25 = pino 6 no esquema WiringPi
    act.WaitingForActivation();
    logMessage("Ativado!");

    logMessage("State machine creation...");
    this->state = State::INIT;
    this->sensors = make_unique<Sensors>();
    this->yoloCtrl = make_unique<YoloCtrl>();

    logMessage("Threads initialization...");

    // Update sensors data and detection data in parallel with the state machine
    if(sensors) sensorThread = thread(&AUVStateMachine::sensorsData, this);
    else throw FailedInitializationSensors();

    if(yoloCtrl) detectionThread = thread(&AUVStateMachine::detectionData, this);
    else throw FailedInitializationYolo();

    logMessage("Threads initialized");
    logMessage("State Machine created");

    this->run();
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
    logMessage("Checking transition...");

    for(const auto& transition : stateTransitions){
        if(transition.lastState == this->lastState && transition.currentState == this->state && transition.targetObject != ""){
            array<int, 4> xyxy = this->yoloCtrl->getXYXY(transition.targetObject);

            if(xyxy[0] != -1){
                this->targetObject = transition.targetObject;
                transitionTo(transition.nextState);
                return true;
            }               
        } else if(transition.nextState == State::SEARCH){
            this->targetObject = transition.targetObject;
            transitionTo(transition.nextState);
            return true;
        }
    }  

    logMessage("Transition not found");

    return false;
}

void AUVStateMachine::transitionTo(State newState){
    logMessage("Transitioning from " + stateToString(this->state) + " to " + stateToString(newState));
    this->lastState = this->state;
    this->state = newState;
}

// END TRANSITION FUNCTIONS

// ERRORS HANDLING

void AUVStateMachine::errorHandling(AUVError e){
    auto* error = dynamic_cast<CollisionDetected*>(&e);

    if(error) if(this->state == State::SEARCH) directionCorrection(error->getAcceleration());
    
    if(dynamic_cast<const FailedConnectThrusters*>(&e) || dynamic_cast<const HighTemperatureError*>(&e)) exit(1); 

    if(dynamic_cast<const ObjectNotFound*>(&e)){
        transitionTo(State::STOP);
        run();
    }
}

void AUVStateMachine::directionCorrection(array<double, 3> acceleration){
    logMessage("Correcting direction...");

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
    logMessage("Searching...");

    // Provavelmente o modo de encontrar os objetos para fazer as transições sera mudado para ser mais especifico para cada caso

    if(this->lastState == State::INIT){
        this->thrusters->defineAction({Action::DOWN, 10});

        sleep_for(seconds(2)); // definir tempo que o AUV demora para descer a uma profundidade onde o gate será visivel

        this->thrusters->defineAction({Action::NONE, 0});

        int rotationCurrent = 0;

        while(!searchObjects("Gate")){
            if(rotationCurrent < 16){
                rotate();
                rotationCurrent++;
            } else throw ObjectNotFound("Gate");
            
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
    // Used if activation is done based on AI
    /*
    while(this->targetObject != OBJECT_INITIALIZATION){
        searchObjects();
        sleep_for(milliseconds(100));
    }
    */

    this->thrusters = make_unique<ThrustersControl>();

    if(thrusters){
        //stabilizesThread = thread(&AUVStateMachine::stabilizes, this);
        checksTransition();
    }
    else throw FailedConnectThrusters();
}  

void AUVStateMachine::passGate(){
    logMessage("Passing gate...");
    int attempts = 0;

    while(attempts < 5) {
        if(centering()){
            while(!searchObjects(INITIALCHOICE)){
                this->thrusters->defineAction({Action::FORWARD, 20}); // Fazer testes para definir essa potencia
                sleep_for(milliseconds(100));
            }
            this->thrusters->defineAction({Action::NONE, 0});

            array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);
            if(xyxy[0] > IMAGE_CENTER[0]) sideIsLeft = false; // Check which side the object is on

            attempts = 0;
            while(attempts < 5){
                if(centering()){
                    double currentDeslocation = 0, distance;

                    array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);
                    calculateDistance(distance, this->targetObject, xyxy);

                    this->thrusters->defineAction({Action::DOWN, 20});
                    sleep_for(seconds(3)); // Definir o tempo necessario para estar abaixo da placa
                    this->thrusters->defineAction({Action::NONE, 0});

                    while(currentDeslocation < distance){
                        int power = 0;
                        distanceSetPower(power, distance - currentDeslocation);

                        this->thrusters->defineAction({Action::FORWARD, power});

                        currentDeslocation += this->sensors->getVel()[0] * this->sensors->deltaTime().count(); // Verificar tamanho da defasagem
                    }
                    this->thrusters->defineAction({Action::NONE, 0});

                    // Rotate after passing through the gate
                    rotate(2 * M_PI, 0.174533, Action::TURNRIGHT);
                    sleep_for(milliseconds(500));
                    rotate(2 * M_PI, 0.174533, Action::TURNRIGHT);
                    
                    checksTransition();
                    break;
                } else {
                    // Tratar a perda do objeto
                    // Verificar ultima posição que ele ainda estava na imagem e voltar para la
                    attempts++;
                }
            }
            break;
        } else{
            // tratar questão de perda de objeto
            // verificar ultima posição que ele ainda estava na imagem e voltar para la
            attempts++;
        }
    }
    if(attempts == 5) throw ObjectNotFound("Gate");
}

// Testar

void AUVStateMachine::alignToPath(){
    logMessage("Aligning to path...");

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
    logMessage("Navigating...");

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
    logMessage("Dropping markers...");

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
    logMessage("Tagging...");

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
    array<double, 3> oriCurrent = this->sensors->getOri();
    array<double, 3> oriInit = oriCurrent;
    Decision decision = {action, 20};
    
    while(fabs(oriCurrent[2] - oriInit[2]) < angle - errorAngle){
        double error = (angle - errorAngle) - fabs(oriCurrent[2] - oriInit[2]);
        decision.value = error * 60;
        this->thrusters->defineAction(decision);

        oriCurrent = this->sensors->getOri();
    }

    decision.value = 0;
    this->thrusters->defineAction(decision);
}

// Testar
// Atualizar para que faça a movimentação correta quando estiver usando a camera de baixo
// Atualizar para que identifique se o objeto esta de lado, se tiver as dimensoes dos objetos da para 
// usar proporção

bool AUVStateMachine::centering(){
    logMessage("Centering...");

    bool isCenter = false;

    while(!isCenter){
        array<int, 4> xyxy = this->yoloCtrl->getXYXY(this->targetObject);

        if(xyxy[0] != -1){
            array<Decision, 2> decision;

            centerObject(decision, xyxy);

            this->thrusters->defineAction(decision[0]);
            this->thrusters->defineAction(decision[1]);
            
            if(decision[0].action == Action::NONE && decision[1].action == Action::NONE) isCenter = true;
        } else{
	        logMessage("Lost object!");
            return isCenter;
        }
    }

    return isCenter;
}

// Falta definir como fazer isso, ainda não temos a garra que segura os markers

void AUVStateMachine::dropping(){

}

// Testar

void AUVStateMachine::advancing(){
    logMessage("Advancing...");

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
            logMessage("Lost object!");
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
    logMessage("Stoping...");

    this->yoloCtrl->stop();
    this->thrusters->finish();
}

// RUN STATE MACHINE

void AUVStateMachine::run(){
    try{
        errorThread = thread(&AUVStateMachine::checksErrors, this);

        while (this->state != State::STOP){
            if(this->state == State::INIT) init();
            else if(this->state == State::SEARCH) search();
            else if(this->state == State::PASSGATE) passGate();
            else if(this->state == State::ALIGNTOPATH) alignToPath();
            else if(this->state == State::NAVIGATE) navigate();
            else if(this->state == State::DROPMARKERS) dropMarkers();
            else if(this->state == State::TAGGING) tagging();

            // Verificar a necessidade
            else if(this->state == State::CENTERING) centering();
            else if(this->state == State::ADVANCING) advancing();
            else if(this->state == State::STABILIZING) stabilizes();
        }

        stop();
    } catch(AUVError e){
        errorHandling(e);
    }
}

// END RUN STATE MACHINE

int main(){
    AUVStateMachine auv;
    return 0;
}
