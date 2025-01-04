#include <iostream>
#include <string>
#include <array>
#include <vector>

using namespace std;

const int MAXDETECTIONS = 5;

struct Object{
    string name;
    int topLeftXY[2];
    int downRightXY[2];
    float confidance;
};

class YoloCtrl{
    private:
        vector<Object> identifiedObjects;
        string host = "0.0.0.0";
        int port = 65432;
        int socket;

    public:
        YoloCtrl(){
            cout << "Object YoloCtrl created" << endl;

            // implementar a conexão

            cout << "Socket opened" << endl;
        }

        void updateData(){
            while(1){
                // implementar o recebimento de dados
            }
        }

        bool foundObject(){
            return !(this->identifiedObjects.empty());
        }

        array<int, 4> getXYXY(string objectName){
            array<int, 4> xyxy = {-1, -1, -1, -1};

            for(vector<Object>::iterator obj = identifiedObjects.begin(); obj != identifiedObjects.end(); obj++){
                if(obj->name == objectName){
                    xyxy[0] = obj->topLeftXY[0];
                    xyxy[1] = obj->topLeftXY[1];
                    xyxy[2] = obj->downRightXY[0];
                    xyxy[3] = obj->downRightXY[1];
                    break;
                }
            }

            return xyxy;
        }

        // Colocar o objeto de maior confiança aqui e tirar a maquina de estados
        string greaterConfidanceObject(){
            Object confidenceObject;

            if(foundObject()){
                // trocar isso por um ordenamento de identifiedObjects, colocando o de maior confiança em 1º
                confidenceObject = *(this->identifiedObjects.begin());

                for(vector<Object>::iterator obj = this->identifiedObjects.begin(); obj != this->identifiedObjects.end(); obj++)
                    if(obj->confidance > confidenceObject.confidance)
                        confidenceObject = *obj;
            }

            return confidenceObject.name;
        }

        void stop(){

        }
};