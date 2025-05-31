/*
Implementação em PYTHON
*/


/**
     * @brief Pre-Processa a Imagem para uma melhor detecção do gate.
     * @details Faz aplicação de filtro gaussiano,Aplicação de melhoria por CLAHE,Converte a imagem de BGR para HSV,muda para escala de cinza e binariza.
     * @param[in] image open CV Mat com a imagem a ser processada.
     * @returns Imagem Pre-Processada.
     */
def PreProcessImage(image):
    return image

/**
     * @brief Recebe uma imagem ja binarizada, acha os contornos e aproxima valores retangulares.
     * @details Utilize o kernel da opencv para achar linhas verticais formando retangulos, limite a 2 para as barras laterais,ache as cordenadas com base no centro da imagem, isso sera rodado em paralelo com a YOLO.
     * @param[in] image open CV Mat ja binarizada.
     * @returns cordenadas uma lista com as coordenadas dos retangulos encontrados.
     */

def DetectGate(image):
    return coordenates[]

/*
-A openCV disponibiliza a maioria das estruturas necessarias para implementação
-A função de preprocessamento pode ser chamada antes ou dentro da detecção de gate mas caso seja chamado dentro o input da imagem não deve ser em binario.
-É necessário que a imagem esteja em escala de cinza e binarizada para a detecção de gate.

*/