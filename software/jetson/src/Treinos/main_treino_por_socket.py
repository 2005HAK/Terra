from socket import socket, AF_INET, SOCK_STREAM
from subprocess import Popen
from json import loads
import matplotlib.pyplot as plt
import time  # só pra garantir um pequeno delay

def main():
    results_dict = []
    server_socket = socket(AF_INET, SOCK_STREAM)
    server_socket.bind(('localhost', 12345))
    server_socket.listen(1)
    print("Programa principal aguardando conexão...")

    num_epochs = int(input("Digite a quantidade de valores a serem recebidos: "))
    epochs_list = [int(input(f"Digite o valor {i+1} de epochs: ")) for i in range(num_epochs)]

    for epoch in epochs_list:
        Popen(["/home/gustavo/Desktop/venvs/venv_terra/bin/python", "Client.py"])
        conn, addr = server_socket.accept()
        print(f"Conectado com {addr}")

        conn.send(str(epoch).encode())
        print(f"Valor de epochs enviado: {epoch}")

        data = conn.recv(1024)
        if data:
            results_dict.append(loads(data.decode()))

        conn.close()
        time.sleep(0.5)  # evitar problemas de concorrência na conexão

    fitness = []
    maps = []
    for i in range(num_epochs):
        fitness.append(results_dict[i]["fitness"])
        maps.append(results_dict[i]["mAP50"])

    server_socket.close()

    plt.figure(figsize=(12, 6))

    # Gráfico de Fitness
    plt.subplot(1, 2, 1)
    plt.plot(epochs_list, fitness, marker='o', color='green', linestyle='-')
    plt.title('Fitness por Número de Epochs')
    plt.xlabel('Epochs')
    plt.ylabel('Fitness')

    # Gráfico de mAP@50
    plt.subplot(1, 2, 2)
    plt.plot(epochs_list, maps, marker='o', color='blue', linestyle='-')
    plt.title('mAP@50 por Número de Epochs')
    plt.xlabel('Epochs')
    plt.ylabel('mAP@50')

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
