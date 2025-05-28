#ifndef YOLOCTRL_H
#define YOLOCTRL_H

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <tuple>
#include "utils.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

const array<int, 4> PINS_STATE_DETECTION = {21, 22, 23, 26}; // GPIO pins for state detection

/**
 * @brief Struct representing an identified object.
 */
struct Object{
    string name;
    double topLeftXY[2];
    double downRightXY[2];
    double confidance;
    int objectId;
    int cam;
};

array<int, 4> decToBin(int decimal);

/**
 * @brief Class responsible for controlling the YOLO object detection.
 */
class YoloCtrl{
    private:
        int server_fd, new_socket;
        struct sockaddr_in address{};
        int opt = 1;
        int addrlen = sizeof(address);
        const char* server_ip = "192.168.0.2";
        int port = 65432;
        int stateDetection = 0;
        int cam = 0;

        vector<Object> identifiedObjects;
        mutex mutexIdentifiedObjects;

        /**
         * @brief Processes the received JSON data and extracts identified objects.
         * 
         * @param received_json The received JSON data.
         * 
         * @return A vector of identified objects.
         */
        vector<Object> process_json(const json& received_json);

    public:
        /**
         * @brief Constructs a new YoloCtrl object.
         */
        YoloCtrl();

        /**
         * @brief Updates the data by fetching new information from the YOLO detection system.
         */
        void updateData();

        /**
         * @brief Switches the camera.
         */
        /*
        void switchCam(int chooseCam);
        */

        /**
         * @brief Checks if any object has been found.
         * 
         * @return True if an object has been found, false otherwise.
         */
        bool foundObject();

        /**
         * @brief Gets the coordinates of the bounding box of the specified object.
         * 
         * @param objectName The name of the object.
         * 
         * @return An array containing the coordinates [x1, y1, x2, y2] of the bounding box.
         */
        array<int, 4> getXYXY(string objectName);

        /**
         * @brief Gets the name of the object with the greatest confidence.
         * 
         * @return The name of the object with the greatest confidence.
         */
        string greaterConfidanceObject();

        bool changeStateDetection(int state);

        void sendStateDetection();

        /**
         * @brief Stops the YOLO control.
         */
        void stop();
};

#endif //YOLOCTRL_H
