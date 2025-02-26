'''
Essa arquivo não está enviando os valores recebidos da IA por socket!
Funcionamento:
    -De modo geral sempre que receber um booleano diferente do que ele tem de base ele altera a YOLO
    -Ele não altera a camera e sim inicia um novo programa com uma camera diferente!
    -O programa que envia os booleanos em cpp é o TradeCamSend.cpp, basicamente a parte que tem que ser substituida é a parte onde eu crio um thread para alterar entre true e false
    ali é onde você deve condicionar a alteração.
'''

import subprocess
from socket import socket, AF_INET, SOCK_STREAM
from struct import unpack
class IA:
    def __init__(self):
        self.cam = 0
        self.comando0 = 'yolo0.py'
        self.comando1 = 'yolo1.py'
        self.running = 0
        self.processo = subprocess.Popen(["python", self.comando0])
        self.HOST = '127.0.0.1'
        self.PORT = 8080
        self.base = True
    def finalizar_programa(self):
        if self.processo is not None:
            try:
                self.processo.terminate()
                self.processo.wait()
            except Exception:
                pass
            self.processo = None

    def start(self):
        self.finalizar_programa()
        if self.running == 0:
            self.processo = subprocess.Popen(["python", self.comando1])
            self.running = 1
        else:
            self.processo = subprocess.Popen(["python", self.comando0])
            self.running = 0

    def run(self):
        while True:
            try:
                client_socket = socket(AF_INET, SOCK_STREAM)
                client_socket.connect((self.HOST, self.PORT))

                try:
                    while True:
                        data = client_socket.recv(1)
                        if not data:
                            break
                        received_bool = unpack("?", data)[0]
                        if received_bool != self.base:
                            self.start()
                            self.base = not self.base

                finally:
                    client_socket.close()
            except Exception:
                print("Cliente não conectado!")

if __name__ == '__main__':
    app = IA()
    app.run()
    app.finalizar_programa()
