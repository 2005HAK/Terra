#include "yoloctrl.h"

// Init class YoloCtrl

YoloCtrl::YoloCtrl(){
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) throw ErrorCreatingSocket();

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if(inet_pton(AF_INET, "0.0.0.0", &address.sin_addr))

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) throw ErrorCreatingSocket();

    // if (listen(server_fd, 3) < 0) throw ErrorListening(); ERRO AQUI

    std::cout << "Server waiting for connection..." << std::endl;
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen); // ERRO AQUI
    if (new_socket < 0) throw ErrorAcceptingConnection();

    std::cout << "Connection established." << std::endl;
    cout << "Object YoloCtrl created." << endl;
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

// Piaz precisa revisar

void YoloCtrl::switchCam(int chooseCam){
    cout << "Switching camera..." << endl;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) throw ErrorCreatingSocket();
    
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        throw InvalidAdreess();
        close(sock);
    }
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw ErrorToConnect();
        close(sock);
        // criar um erro pra isso
    }
    
    int32_t net_number = htonl(chooseCam); 
    send(sock, &net_number, sizeof(net_number), 0);
    close(sock);

    if(cam == chooseCam){
        std::cout << "Value changed to: " << (cam == 0 ? "Front camera" : "Bottom camera") << std::endl;
    } else{
        // gerar erro
    }
}

vector<Object> YoloCtrl::process_json(const json& received_json){
    vector<Object> results;
    try {
        if (received_json.count("data")) {  // 'contains' não existia antes do JSON 3.9.0
            for (const auto& obj : received_json["data"]) {
                if (obj.is_array() && obj.size() >= 6) {
                    Object currentObject;
                    currentObject.topLeftXY[0] = obj[0];
                    currentObject.topLeftXY[1] = obj[1];
                    currentObject.downRightXY[0] = obj[2];
                    currentObject.downRightXY[1] = obj[3];
                    currentObject.confidance = obj[4];
                    currentObject.objectId = obj[5];
                    currentObject.name = "Unknown";

                    if (received_json.count("names") && received_json["names"].count(to_string(currentObject.objectId))) {
                        currentObject.name = received_json["names"][to_string(currentObject.objectId)];
                    }
                    results.emplace_back(currentObject);
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

    for(const auto& obj : identifiedObjects){
        if(obj.name == objectName){
            xyxy[0] = obj.topLeftXY[0];
            xyxy[1] = obj.topLeftXY[1];
            xyxy[2] = obj.downRightXY[0];
            xyxy[3] = obj.downRightXY[1];
            break;
        }
    }
    return xyxy;
}

// Colocar o objeto de maior confiança aqui e tirar a maquina de estados
string YoloCtrl::greaterConfidanceObject(){
    lock_guard<mutex> lock(mutexIdentifiedObjects);

    if(identifiedObjects.empty()) return "";

    Object confidenceObject = identifiedObjects[0];
    for(const auto& obj : identifiedObjects) if(obj.confidance > confidenceObject.confidance) confidenceObject = obj;

    return confidenceObject.name;
}

void YoloCtrl::stop(){
    close(new_socket);
    close(server_fd);
}

// End class ToloCtrl
