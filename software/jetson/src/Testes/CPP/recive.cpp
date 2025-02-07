#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;
using json = nlohmann::json;
using namespace std;

void process_json(const json& received_json) {
    try {
        if (received_json.contains("data")) {
            cout << "\nDetectados: " << received_json["data"].size() << " objetos." << endl;
            for (const auto& ob j : received_json["data"]) {
                if (obj.is_array() && obj.size() >= 6) {
                    cout << "\nObjeto detectado:" << endl;
                    cout << " - Posição: (" << obj[0] << ", " << obj[1] << ")" << endl;
                    cout << " - Dimensão: (" << obj[2] << " x " << obj[3] << ")" << endl;
                    cout << " - Confiança: " << obj[4] << endl;
                    int class_id = obj[5];
                    cout << " - Classe ID: " << class_id;
                    if (received_json.contains("names") && received_json["names"].contains(to_string(class_id))) {
                        cout << " (" << received_json["names"][to_string(class_id)] << ")";
                    }
                    cout << endl;
                }
            }
        } else {
            cout << "Nenhum dado de detecção encontrado." << endl;
        }
    } catch (const json::exception& e) {
        cerr << "Erro ao processar JSON: " << e.what() << endl;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 65432));
        cout << "Servidor aguardando conexões na porta 65432..." << endl;

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
                cout << "\nDados recebidos:" << endl;
                cout << received_json.dump(4) << endl;
                process_json(received_json);
            } catch (const json::parse_error& e) {
                cerr << "Erro ao interpretar JSON: " << e.what() << endl;
            }
        }
    } catch (exception& e) {
        cerr << "Erro: " << e.what() << endl;
    }
    return 0;
}
