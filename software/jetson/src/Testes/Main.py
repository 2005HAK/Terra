from LibraryYOLO import Verificar_Cuda,Definir_Modelo,Definir_Host,Rodar_Yolo
Verificar_Cuda()
model = Definir_Modelo()
PORT = 65432
HOST = Definir_Host()
Rodar_Yolo(HOST,PORT,model)