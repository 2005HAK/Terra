#ifndef YOLOCTRL_H
#define YOLOCTRL_H

#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <variant>
#include <vector>
#include <tuple>
#include <mutex>
#include "utils.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

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
	mutex mutexIdentifiedObjects;

        vector<Object> process_json(const json& received_json);

    public:
        YoloCtrl();

        void updateData();


        bool foundObject();

        array<int, 4> getXYXY(string objectName);

        string greaterConfidanceObject();

        void stop();
};

#endif //YOLOCTRL_H
