from LibraryYOLO import Verificar_Cuda,Definir_Modelo,Definir_Host,Rodar_Yolo
PORT = 65432
print(f"Rodando Em: {Verificar_Cuda()}")
model = Definir_Modelo()
HOST = Definir_Host()
Rodar_Yolo(HOST,PORT,model)