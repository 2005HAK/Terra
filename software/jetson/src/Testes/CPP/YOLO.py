'''

Dependencias de recebimento(Linux):

sudo apt update
sudo apt install libboost-all-dev
sudo apt install nlohmann-json3-dev
g++ -o server recive.cpp -lboost_system -lpthread

O recive2.cpp tem o ultimo código que interpreta melhor

'''
from ultralytics import YOLO
from socket import socket,AF_INET,SOCK_STREAM
from json import dumps
class IA:
    def __init__(self):
        self.source=0
        self.verbose=False
        self.stream=True
        self.show=True
        self.imgsz=640
        self.save=False
        self.max_det=1
        self.conf=.8
        self.HOST = '192.168.1.65'
        self.PORT = 65432
        self.model=YOLO('/home/hak/Documents/Terra/software/jetson/src/Testes/Treinos/modelos/TreinoCuboV2Para160px01.pt')

    def enviar_dicionario(self,dicio):
        try:
            with socket(AF_INET, SOCK_STREAM) as s:
                s.connect((self.HOST, self.PORT))
                data_json = dumps(dicio) + '\n'
                s.sendall(data_json.encode('utf-8'))
        except ConnectionAbortedError as e:
            print(f"Conexão abortada: {e}")
        except Exception as e:
            print(f"Erro inesperado: {e}")

    def run(self):
        while True:
            for results in self.model.predict(source=self.source, verbose=self.verbose, stream=self.stream, show=self.show, imgsz=self.imgsz, save=self.save,max_det=self.max_det, conf=self.conf):
                for result in results:
                    boxes = result.boxes.data.cpu().numpy().tolist()
                    dicio = {'data': boxes, 'names': result.names}
                    self.enviar_dicionario(dicio)

if __name__ == '__main__':
    ia = IA()
    ia.run()