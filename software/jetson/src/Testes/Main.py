from LibraryYOLO import Verificar_Cuda,Definir_Modelo,Definir_Host,Rodar_Yolo,modo,Teste_Confianca
Verificar_Cuda()
model = Definir_Modelo()
if modo() == 1:
    Teste_Confianca(model)
else:
    PORT = 65432
    HOST = Definir_Host()
    Rodar_Yolo(HOST,PORT,model)