#include "thrustersctrl.h"

int main(){
    ThrustersControl thrusters;

    int percent = 0;

    while(true){
        cout << "Digite o valor: " << endl;
        cin >> percent;
        thrusters.defineAction({Action::FORWARD, percent});
    }
    
    return 0;
}