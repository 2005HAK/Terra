#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <variant>
#include <vector>
#include <tuple>
#include "utils.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

using VarType = std::variant<double, int, std::string>;

struct Object{
    string name;
    double topLeftXY[2];
    double downRightXY[2];
    double confidance;
    int objectId;
};

class YoloCtrl{
    private:
        vector<Object> identifiedObjects;

    public:
        YoloCtrl(){
            cout << "Object YoloCtrl created" << endl;
        }

        void updateData(){
            while(1){
                receive();
                sleep_for(milliseconds(350));
            }
        }

        void process_json(const json& received_json) {
            try{
                if(received_json.contains("data")){
                    for(const auto& obj : received_json["data"]){
                        Object currentObject;

                        if(obj.is_array() && obj.size() >= 6){
                            currentObject.topLeftXY[0] = obj[0];
                            currentObject.topLeftXY[1] = obj[1];
                            currentObject.downRightXY[0] = obj[2];
                            currentObject.downRightXY[1] = obj[3];
                            currentObject.confidance = obj[4];
                            currentObject.objectId = obj[5];
                            currentObject.name = "Unknown";

                            if(received_json.contains("names") && received_json["names"].contains(to_string(currentObject.objectId))) 
                                currentObject.name = received_json["names"][to_string(currentObject.objectId)];

                            identifiedObjects.emplace_back(currentObject);
                        }
                    }
                }
            }catch (const json::exception& e){
                cerr << "Erro ao processar JSON: " << e.what() << endl;
            }
        }
    
        void receive(){  
            try{
                boost::asio::io_context io_context;
                tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 65432));
    
                tcp::socket socket(io_context);
                acceptor.accept(socket);

                boost::asio::streambuf buffer;
                boost::asio::read_until(socket, buffer, '\n');

                istream is(&buffer);
                string received_data;
                getline(is, received_data);

                try {
                    json received_json = json::parse(received_data);
                    
                    process_json(received_json);
                } catch (const json::parse_error& e) {
                    cerr << "Erro ao interpretar JSON: " << e.what() << endl;
                }
                
            } catch (exception& e) {
                cerr << "Erro: " << e.what() << endl;
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
