/*
Utilizar as variavei obj_ que estão sendo printadas na main
Se precisar que fique mais rapido pode retirar diretamente da variavel json(mas é meio chato de interpretar)
Oque eu recomendaria é utilizar desse mesmo código pra interpratação
(só mudar o nome das funções) e talvez colocar em um arquivo .h, mas ai depende de como o cpp ta implementado
*/
#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;
using json = nlohmann::json;
using namespace std;

vector<tuple<double, double, double, double, double, int, string>> process_json(const json& received_json) {
    vector<tuple<double, double, double, double, double, int, string>> results;
    try {
        if (received_json.contains("data")) {
            for (const auto& obj : received_json["data"]) {
                if (obj.is_array() && obj.size() >= 6) {
                    double x_min = obj[0];
                    double y_min = obj[1];
                    double x_max = obj[2];
                    double y_max = obj[3];
                    double confidence = obj[4];
                    int class_id = obj[5];
                    string class_name = "Unknown";

                    if (received_json.contains("names") && received_json["names"].contains(to_string(class_id))) {
                        class_name = received_json["names"][to_string(class_id)];
                    }
                    results.emplace_back(x_min, y_min, x_max, y_max, confidence, class_id, class_name);
                }
            }
        }
    } catch (const json::exception& e) {
        cerr << "Erro ao processar JSON: " << e.what() << endl;
    }
    return results;
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 65432));

        // Variáveis para armazenar os resultados
        vector<tuple<double, double, double, double, double, int, string>> results;

        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            boost::asio::streambuf buffer;
            boost::asio::read_until(socket, buffer, '\n');

            istream is(&buffer);
            string received_data;
            getline(is, received_data);

            try {
                json received_json = json::parse(received_data);

                results = process_json(received_json);

                // Mover o cout para dentro do loop para garantir que os valores corretos sejam impressos
                for (const auto& [x_min, y_min, x_max, y_max, confidence, class_id, class_name] : results) {
                    double obj_x_min = x_min;
                    double obj_y_min = y_min;
                    double obj_x_max = x_max;
                    double obj_y_max = y_max;
                    double obj_confidence = confidence;
                    int obj_class_id = class_id;
                    string obj_class_name = class_name;

                    // Agora, imprimir os valores
                    cout << "\nObjeto detectado:" << endl;
                    cout << " - Posição: (" << obj_x_min << ", " << obj_y_min << ")" << endl;
                    cout << " - Dimensão: (" << obj_x_max << " x " << obj_y_max << ")" << endl;
                    cout << " - Confiança: " << obj_confidence << endl;
                    cout << " - Classe ID: " << obj_class_id << " (" << obj_class_name << ")" << endl;

                }
            } catch (const json::parse_error& e) {
                cerr << "Erro ao interpretar JSON: " << e.what() << endl;
            }
        }
    } catch (exception& e) {
        cerr << "Erro: " << e.what() << endl;
    }
    return 0;
}
