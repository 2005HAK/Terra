#ifndef RECEIVER_H
#define RECEIVER_H

#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <variant>
#include <vector>
#include <tuple>
#include <string>

using boost::asio::ip::tcp;
using json = nlohmann::json;
using namespace std;

using VarType = std::variant<double, int, std::string>;

class Receiver {
public:
    vector<tuple<double, double, double, double, double, int, string>> process_json(const json& received_json) {
        vector<tuple<double, double, double, double, double, int, string>> results;
        try {
            if (received_json.count("data")) {  // 'contains' não existia antes do JSON 3.9.0
                for (const auto& obj : received_json["data"]) {
                    if (obj.is_array() && obj.size() >= 6) {
                        double x_min = obj[0];
                        double y_min = obj[1];
                        double x_max = obj[2];
                        double y_max = obj[3];
                        double confidence = obj[4];
                        int class_id = obj[5];
                        string class_name = "Unknown";

                        if (received_json.count("names") && received_json["names"].count(to_string(class_id))) {
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

    vector<VarType> receive() {
        vector<VarType> final_results;

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

                    vector<tuple<double, double, double, double, double, int, string>> results = process_json(received_json);

                    for (const auto& [x_min, y_min, x_max, y_max, confidence, class_id, class_name] : results) {
                        final_results.push_back(x_min);
                        final_results.push_back(y_min);
                        final_results.push_back(x_max);
                        final_results.push_back(y_max);
                        final_results.push_back(confidence);
                        final_results.push_back(class_id);
                        final_results.push_back(class_name);
                    }
                } catch (const json::parse_error& e) {
                    cerr << "Erro ao interpretar JSON: " << e.what() << endl;
                }
            }

        } catch (exception& e) {
            cerr << "Erro: " << e.what() << endl;
        }
        return final_results;
    }
};

#endif
/*
Para acessar os valores de final_results utilize:
get <double>(final_results[pos])

Double deve ser substituído pelo tipo da variável.
Os valores serão escritos no final_results da seguinte maneira:
[0] Double x_min
[1] Double y_min
[2] Double x_max
[3] Double y_max
[4] Double confidence
[5] int class_id
[6] string class_name
*/
