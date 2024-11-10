from socket import socket,AF_INET,SOCK_STREAM
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
                data="datasets/data",
                epochs=epochs,
                imgsz=640,
                name=f"Treino_Com_{epochs}_Epochs",
                project="TreinosCubo",
                save=True
            )

        results_dict = {
            "maps_50": results.maps["50"],
            "val_loss": results.results_dict["metrics/val/loss"]
        }

        client_socket.send(dumps(results_dict).encode())
        print("Dicionário de resultados enviado!")

    client_socket.close()

if __name__ == "__main__":
    main()