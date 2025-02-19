#include <iostream>
#include "/home/equipeterra/Terra/software/jetson/src/Testes/CPP/reciveobj2.h"
using namespace std;

int main()
{
    Receiver recebidor;
    /*Cria um reciver*/
    while (true)
    {
        vector<VarType> dados = recebidor.receive();
        /*Recebe os dados*/

        try
        {
            cout << "x_min: "<< get<double>(dados[0]) << endl;
            cout << "y_min: "<< get<double>(dados[1]) << endl;
            cout << "x_max: "<< get<double>(dados[2]) << endl;
            cout << "y_max: "<< get<double>(dados[3]) << endl;
            cout << "Confianca: "<< get<double>(dados[4]) << endl;
            cout << "ID: "<< get<int>(dados[5]) << endl;
            cout << "Classe: "<< get<string>(dados[6]) << endl<<endl;
            /*Printa os dados*/
        }
        catch (const bad_variant_access &e)
        {
            cerr << "Erro ao acessar variante: " << e.what() << endl;
        }
    }

    return 0;
}
/*
Para acessar os valores de final results utilize: 
get <double>(final_results[pos])

Double deve ser substituido pelo tipo da variavel
Os valores serão escritos no final results da seguinte maneira:
[0]Double x_min
[1]Double y_min
[2]Double x_max
[3]Double y_max
[4]Double confidence
[5]int class_id
[6]string class_name
*/
