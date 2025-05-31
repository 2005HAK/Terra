from socket import socket, AF_INET, SOCK_STREAM
from json import dumps
from ultralytics import YOLO

def main():
    client_socket = socket(AF_INET, SOCK_STREAM)
    client_socket.connect(('localhost', 12345))
    print("Programa 2 aguardando receber valor de epochs...")

    data = client_socket.recv(1024)
    if data:
        epochs = int(data.decode())
        print(f"Treinando modelo com {epochs} epochs...")

        model = YOLO("yolo11n.pt")
        results = model.train(
            device='cuda',
            data="/home/gustavo/Desktop/git/Terra/software/jetson/src/Treinos/data.yaml",
            epochs=epochs,
            imgsz=160,
            name=f"Treino_Com_{epochs}_Epochs",
            project="../modelos",
            save=True
        )
        
        print(f"Dicionario de results:\n{results.results_dict}")
        
        results_dict = {
            "precision": results.results_dict['metrics/precision(B)'],
            "recall": results.results_dict['metrics/recall(B)'],
            "mAP50": results.results_dict['metrics/mAP50(B)'],
            "mAP50_95": results.results_dict['metrics/mAP50-95(B)'],
            "fitness": results.results_dict['fitness']
        }

        client_socket.send(dumps(results_dict).encode())
        print("Dicionário de resultados enviado!")

    client_socket.close()

if __name__ == "__main__":
    main()
