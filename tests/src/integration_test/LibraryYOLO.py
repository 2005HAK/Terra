from ultralytics import YOLO
from cv2 import VideoCapture
from torch.cuda import is_available
from json import dumps
import socket
from os import listdir
import time

def Verificar_Cuda():
    device = 'cuda' if is_available() else 'cpu'
    return device.upper()

def Enviar_Dicionario(HOST, PORT, dicio):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            data_json = dumps(dicio)
            s.sendall(data_json.encode('utf-8'))
    except ConnectionAbortedError as e:
        print(f"Conexão abortada: {e}")
    except Exception as e:
        print(f"Erro inesperado: {e}")

def Definir_Modelo():
    # Modelos = listdir('modelos')
    # x=0
    # print("Qual Versão Gostaria de Escolher?")
    # for mod in Modelos:
    #     print("[",x,"][",mod,"]")
    #     x+=1
    # esc = int(input())
    model = YOLO("yolov8n.pt") # YOLO('modelos/'+Modelos[esc])
    return model

def Definir_Host():
    x = input("Usar o HOST Padrão?(s/n) ")
    if x == 's':
        return '192.168.1.2'
    else:
        HOST = str(input("Digite o ip host:"))
        return HOST

def Verificar_Transmissao():
    v = input("Conectar com servidor?(s/n) ")
    if v == 's': return True
    else: return False

def Rodar_Yolo(HOST,PORT,model):
    conex = Verificar_Transmissao()
    cap = VideoCapture(0)
    if conex:
        print(f"Conectado ao servidor {HOST} na porta {PORT}")
    px = int(input("Rodar Em quantos Pixels? "))

    

    while True:
        old = time.time()
        frames = []

        while len(frames) < 10:
            ret, frame = cap.read()
            if not ret:
                break

            frames.append(frame)
        
        new = time.time()

        print(new - old)       

        if not ret:
            break
        results = model(frames, verbose=False, stream=True, show=False,imgsz=px,save=False,max_det=1)
        
        for result in results:
            boxes = result.boxes.data.cpu().numpy().tolist()
            dicio = {'data': boxes, 'names': result.names}
            if conex:
                Enviar_Dicionario(HOST,PORT,dicio)
        