import threading
import psutil
import os
import time
import subprocess
import matplotlib.pyplot as plt

# Função para coletar o uso de memória de um processo específico a cada 1 segundo
def collect_memory_usage(auv_process, process, memory_data, interval=1):
    while auv_process.poll() is None:
        memory_usage = process.memory_info().rss
        memory_data.append(memory_usage / 1024 / 1024)  # Converte para MB
        time.sleep(interval)

def main():
    # Objeto para armazenar os dados de memória
    memory_data = []

    # Inicializa o AUV em um subprocesso
    auv_process = subprocess.Popen(['python', 'navegation.py'])  # Substitua pelo script do AUV

    # Cria e inicia a thread para coletar o uso de memória do AUV em paralelo
    auv_pid = auv_process.pid  # Obtém o PID do processo do AUV
    process = psutil.Process(auv_pid)

    # Coleta a memória em paralelo
    memory_thread = threading.Thread(target=collect_memory_usage, args=(auv_process, process, memory_data, 1))
    memory_thread.start()

    # Aguarda a execução do AUV
    auv_process.wait()  # Espera o subprocesso do AUV terminar

    # Aguarda a thread de coleta de memória terminar
    memory_thread.join()

    # Plota o gráfico de uso de memória
    plt.plot(memory_data)
    plt.title("Uso de Memória RAM durante a Execução do AUV")
    plt.xlabel("Tempo (segundos)")
    plt.ylabel("Uso de Memória (MB)")
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    main()
