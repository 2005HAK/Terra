from ultralytics import YOLO
from ultralytics.utils.checks import cuda_is_available
from json import dumps
from socket import socket,AF_INET,SOCK_STREAM
from os import listdir

def Verificar_Cuda():
    print("Rodando em: CUDA" if cuda_is_available() else "Rodando em: CPU")

def Enviar_Dicionario(HOST, PORT, dicio):
    try:
        with socket(AF_INET, SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            data_json = dumps(dicio)
            s.sendall(data_json.encode('utf-8'))
    except ConnectionAbortedError as e:
        print(f"Conexão abortada: {e}")
    except Exception as e:
        print(f"Erro inesperado: {e}")

def Definir_Modelo():
    Modelos = listdir('Treinos/modelos')
    x=0
    print("Qual Versão Gostaria de Escolher?")
    for mod in Modelos:
        print("[",x,"][",mod,"]")
        x+=1
    esc = int(input())
    return YOLO('Treinos/modelos/'+Modelos[esc])

def Definir_Host():
    x = input("Usar o HOST Padrão?(s/n) ")
    if x == 's':
        return '192.168.0.1'
    else:
        return str(input("Digite o ip host:"))

def Rodar_Yolo(HOST,PORT,model):
    print(f"Conectado ao servidor {HOST} na porta {PORT}")
    px = int(input("Rodar Em quantos Pixels? "))
    while True:
        for results in model.predict(source=0, verbose=False, stream=True, show=True,imgsz=px,save=False,max_det=1, conf=.8):
            for result in results:
                boxes = result.boxes.data.cpu().numpy().tolist()
                dicio = {'data': boxes, 'names': result.names}
                Enviar_Dicionario(HOST,PORT,dicio)

