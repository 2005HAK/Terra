#include <array>
#include <chrono>

using namespace std;
using namespace chrono;

class Sensors{
    private:
        array<float, 3> acc = {0, 0, 0};
        array<float, 3> gyro = {0, 0, 0};
        array<float, 3> mag = {0, 0, 0};
        array<float, 3> vel = {0, 0, 0};
        float tempPixhawk;
        float tempRaspberry;
        steady_clock::time_point currentTime;
        steady_clock::time_point oldTime;

    public:
        Sensors(){
            // Conexão

            steady_clock::time_point currentTime = steady_clock::now();
            steady_clock::time_point oldTime = currentTime;

            sendHeartbeat();
        }

        void sendHeartbeat(){

        }

        void updateData(){
            while(1){

            }
        }

        void collisionDetect(){

        }

        void detectOverheat(){

        }
};