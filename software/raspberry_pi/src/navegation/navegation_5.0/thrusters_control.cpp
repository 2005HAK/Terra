#include <vector>
#include <string>

using namespace std;

enum class Action{
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT,
    UP,
    DOWN,
    TURNRIGHT,
    TURNLEFT,
    STAY
};

string actionToString(Action action){
    switch (action){
        case Action::FORWARD:
            return "FORWARD";
        case Action::BACKWARD:
            return "BACKWARD";
        case Action::RIGHT:
            return "RIGHT";
        case Action::LEFT:
            return "LEFT";
        case Action::UP:
            return "UP";
        case Action::DOWN:
            return "DOWN";
        case Action::TURNRIGHT:
            return "TURNRIGHT";
        case Action::TURNLEFT:
            return "TURNLEFT";
        case Action::STAY:
            return "STAY";
        default:
            return "UNKNOWN";
    }
}

class ThrustersControl{
    public:
        ThrustersControl(){

        }

        void defineAction(Action action, int value){
            cout << "Action: " + actionToString(action) << ", Power: " + value << endl;
            switch (action){
                case Action::UP:
                    break;
                case Action::DOWN:
                    break;
                case Action::FORWARD:
                    break;
                case Action::BACKWARD:
                    break;
                case Action::LEFT:
                    break;
                case Action::RIGHT:
                    break;
                case Action::TURNLEFT:
                    break;
                case Action::TURNRIGHT:
                    break;
                case Action::STAY:
                    break;
            }
        }
};