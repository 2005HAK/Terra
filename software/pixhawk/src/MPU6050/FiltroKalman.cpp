#include <iostream> 
#include <vector> 
#include <cmath>
#include <chrono>

//----------Variáveis----------
    //# Ruido = r
    //# Erro de medição = erm
    //# Estimativa inicial = ei
    //# Variância_da_estimativa = ve
    //# Variância_da_estimativa_extrapolada = vx

std::vector<float> filtro_Kalman(std::vector<float> vector){

    std::vector<float> newVector;
    float r = 0.05, erm = 0.1, ve = 0.5;

    // X axis 
    float eix = 1;
    float vxx = pow(ve, 2) + r;
    float gkx = 0, estadoAtualx = 0, variacaoestadoAtualx = 0;

    //#Estimativa do estado atual
    gkx = vxx / float(vxx + erm); //#Ganho de Kalman
    estadoAtualx = eix + gkx*(vector[0] - eix);
    variacaoestadoAtualx = (1 - gkx)*vxx;

    //#Previsão
    eix = estadoAtualx;
    vxx = variacaoestadoAtualx + r;
    newVector.push_back(eix);


    // Y axis 
    float eiy = 1;
    float vxy = pow(ve, 2) + r;
    float gky = 0, estadoAtualy = 0, variacaoestadoAtualy = 0;

    //#Estimativa do estado atual
    gky = vxy / float(vxy + erm); //#Ganho de Kalman
    estadoAtualy = eiy + gky*(vector[1] - eiy);
    variacaoestadoAtualy = (1 - gky)*vxy;

    //#Previsão
    eiy = estadoAtualy;
    vxy = variacaoestadoAtualy + r;
    newVector.push_back(eiy);


    // Z axis 
    float eiz = 1;
    float vxz = pow(ve, 2) + r;
    float gkz = 0, estadoAtualz = 0, variacaoestadoAtualz = 0;

    //#Estimativa do estado atual
    gkz = vxz / float(vxz + erm); //#Ganho de Kalman
    estadoAtualz = eiz + gkz*(vector[2] - eiz);
    variacaoestadoAtualz = (1 - gkz)*vxz;

    //#Previsão
    eiz = estadoAtualz;
    vxz = variacaoestadoAtualz + r;
    newVector.push_back(eiz);

    return newVector;
}

int main(){
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<float> vet, nvet;

  vet.push_back(1.0);
  vet.push_back(0.8);
  vet.push_back(1.3);

  nvet = filtro_Kalman(vet);

  for(float valor : nvet){
    std::cout << valor << " ";
  }

  auto end = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "\nTempo de execução: " << duration.count() << std::endl;
}