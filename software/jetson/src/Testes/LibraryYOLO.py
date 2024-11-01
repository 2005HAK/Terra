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
    Modelos = listdir('modelos')
    x=0
    print("Qual Versão Gostaria de Escolher?")
    for mod in Modelos:
        print("[",x,"][",mod,"]")
        x+=1
    esc = int(input())
    return YOLO('modelos/'+Modelos[esc])

def Definir_Host():
    x = input("Usar o HOST Padrão?(s/n) ")
    if x == 's':
        return '192.168.0.1'
    else:
        return str(input("Digite o ip host:"))
def Verificar_Transmissao():
    v = input("Conectar com servidor?(s/n) ")
    if v == 's': return True
    else: return False

def Rodar_Yolo(HOST,PORT,model):
    conex = Verificar_Transmissao()
    if conex:
        print(f"Conectado ao servidor {HOST} na porta {PORT}")
    px = int(input("Rodar Em quantos Pixels? "))
    while True:
        for results in model.predict(source=0, verbose=False, stream=True, show=True,imgsz=px,save=False,max_det=1, conf=.8):
            for result in results:
                boxes = result.boxes.data.cpu().numpy().tolist()
                dicio = {'data': boxes, 'names': result.names}
                if conex:
                    Enviar_Dicionario(HOST,PORT,dicio)

def Teste_Confianca(model):
    st = bool(input("Stream?(False/True) "))
    sh = bool(input("Show?(False/True) "))
    hf = bool(input("Half?(False/True) "))
    sv = bool(input("Save?(False/True) "))
    px = int(input("Quantidade de Pixels? "))
    results = model('Video.mp4', stream=st, show=sh, half=hf, save=sv,imgsz=px,max_det=1)
    i = 0
    mediac = 0
    for result in results:
        for datas in result.boxes.data:
            i += 1
            mediac += datas[4]
            print(f"Confiança: {datas[4] * 100:.4}%")
    print(f"A Media de confiança é: {(mediac / i) * 100:.4}")
    print(f"Quantidade de detecções: {i}")
def modo():
    return int(input("Qual Modo gostaria de escolher?\n[1][Teste de Confiança]\n[2][Run em SSH]\n"))