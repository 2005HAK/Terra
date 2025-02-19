#include "yoloctrl.h"

// Init class YoloCtrl

YoloCtrl::YoloCtrl(){
    cout << "Object YoloCtrl created" << endl;
}

void YoloCtrl::updateData(){
    lock_guard<mutex> lock(mutexIdentifiedObjects);

    try {
        boost::asio::io_service io_service;  // io_context substituído por io_service
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 65432));

        tcp::socket socket(io_service);
        acceptor.accept(socket);

        boost::asio::streambuf buffer;
        boost::system::error_code error;
        boost::asio::read_until(socket, buffer, '\n', error);

        if (!error || error == boost::asio::error::eof) {  // EOF tratado corretamente
            istream is(&buffer);
            string received_data;
            getline(is, received_data);

            try {
                json received_json = json::parse(received_data);

                identifiedObjects = process_json(received_json);

            } catch (const json::parse_error& e) {
                cerr << "Erro ao interpretar JSON: " << e.what() << endl;
            }
        }

    } catch (exception& e) {
        cerr << "Erro: " << e.what() << endl;
    }
}

vector<Object> YoloCtrl::process_json(const json& received_json){
    vector<Object> results;
    try {
        if (received_json.count("data")) {  // 'contains' não existia antes do JSON 3.9.0
            for (const auto& obj : received_json["data"]) {
                if (obj.is_array() && obj.size() >= 6) {
		    cout << obj << endl;
                    Object currentObject;
                    currentObject.topLeftXY[0] = obj[0];
                    currentObject.topLeftXY[0] = obj[1];
                    currentObject.downRightXY[0] = obj[2];
                    currentObject.downRightXY[0] = obj[3];
                    currentObject.confidance = obj[4];
                    currentObject.objectId = obj[5];
                    currentObject.name = "Unknown";

                    if (received_json.count("names") && received_json["names"].count(to_string(currentObject.objectId))) {
                        currentObject.name = received_json["names"][to_string(currentObject.objectId)];
                    }
                    results.emplace_back(currentObject);

                    cout << "Name: " << currentObject.name << endl;
                    cout << "Xmin: " << currentObject.topLeftXY[0] << endl;
                    cout << "Ymin: " << currentObject.topLeftXY[1] << endl;
                    cout << "Xmax: " << currentObject.downRightXY[0] << endl;
                    cout << "Ymax: " << currentObject.downRightXY[1] << endl;
                    cout << "Conf: " << currentObject.confidance << endl;
                }
            }
        }
    } catch (const json::exception& e) {
        cerr << "Erro ao processar JSON: " << e.what() << endl;
    }
    return results;
}

bool YoloCtrl::foundObject(){
    return !(this->identifiedObjects.empty());
}

array<int, 4> YoloCtrl::getXYXY(string objectName){
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
string YoloCtrl::greaterConfidanceObject(){
    lock_guard<mutex> lock(mutexIdentifiedObjects);

    if(identifiedObjects.empty()) return "";
    else{
        Object confidenceObject = identifiedObjects[0];

        for(const auto& obj : identifiedObjects)
            if(obj.confidance > confidenceObject.confidance)
                confidenceObject = obj;

        return confidenceObject.name;
    }
}

void YoloCtrl::stop(){

}

// End class ToloCtrl
