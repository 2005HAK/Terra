'''

Dependencias de recebimento (Linux):

sudo apt update
sudo apt install libboost-all-dev
sudo apt install nlohmann-json3-dev
g++ -o server recive.cpp -lboost_system -lpthread

O recive2.cpp tem o último código que interpreta melhor

'''
from ultralytics import YOLO
from socket import socket, AF_INET, SOCK_STREAM
from json import dumps
import sys

class IA:
    def __init__(self, cam_index):
        self.source = int(cam_index)
        self.verbose = False
        self.stream = True
        self.show = True
        self.imgsz = 640
        self.save = False
        self.max_det = 3
        self.conf = 0.8
        self.HOST = '192.168.0.1' # Endereço IP da raspberry
        self.PORT = 65432
        self.model = YOLO('../modelos/SHARK_SAWFISH.pt')  # Carrega o modelo YOLOv8

    def enviar_dicionario(self, dicio):
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
            for results in self.model.predict(
                source=self.source, verbose=self.verbose, stream=self.stream, 
                show=self.show, imgsz=self.imgsz, save=self.save, 
                max_det=self.max_det, conf=self.conf
            ):
                for result in results:
                    boxes = result.boxes.data.cpu().numpy().tolist()
                    dicio = {
                        'data': [box[:5] + [int(box[5])] for box in boxes],  # Corrige class_id para int
                        'names': result.names,
                        'cam': self.source
                    }
                    self.enviar_dicionario(dicio)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Uso: python run.py --0 ou python run.py --1")
        sys.exit(1)
    
    if sys.argv[1] == "--0":
        cam_index = 0
    elif sys.argv[1] == "--1":
        cam_index = 1
    else:
        print("Opção inválida! Use --0 para câmera 0 ou --1 para câmera 1")
        sys.exit(1)
    
    ia = IA(cam_index)
    ia.run()
