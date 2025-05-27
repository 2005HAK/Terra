from subprocess import Popen
from os import kill
from signal import SIGKILL
import socket

class Alternador:
    def __init__(self):
        self.cam = 0
        self.program0 = ["python", "run.py", "--0"]
        self.program1 = ["python", "run.py", "--1"]
        self.process = Popen(self.program0)

    def finalizar_processo(self):
        if self.process and self.process.poll() is None:
            self.process.terminate()
            self.process.wait(timeout=2)  
            if self.process.poll() is None:  
                kill(self.process.pid, SIGKILL)
                print("Programa foi forçado a encerrar")
            else:
                print("Programa finalizado com sucesso")

    def iniciar_programa(self, program):
        self.process = Popen(program)
        print(f"Iniciando {program}")

    def trade(self):
        novo_programa = self.program1 if self.cam == 0 else self.program0
        self.finalizar_processo()
        self.iniciar_programa(novo_programa)
        self.cam = 1 - self.cam 

    def receive_int(self, host='0.0.0.0', port=12345):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
            server_socket.bind((host, port))
            server_socket.listen(1)
            print(f"Aguardando conexão em {host}:{port}...")

            while True:
                conn, addr = server_socket.accept()
                with conn:
                    print(f"Conectado por {addr}")
                    data = conn.recv(4)
                    if len(data) == 4:
                        received_number = int.from_bytes(data, byteorder='big', signed=True)
                        if received_number != self.cam:
                            self.trade()
                    else:
                        print("Erro ao receber dados.")

if __name__ == '__main__':
    alt = Alternador()
    alt.receive_int()
