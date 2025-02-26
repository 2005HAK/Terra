import tkinter as tk
import subprocess
import os
import signal
class IA:
    def __init__(self):
        self.root = tk.Tk()
        self.root.geometry("400x400")
        self.root.title("Teste Troca Camera")
        self.cam = 0
        self.comando0 = 'yolo0.py'
        self.comando1 = 'yolo1.py'
        self.running = 0
        self.processo = subprocess.Popen(["python", self.comando0])

    def finalizar_programa(self):
        if self.processo is not None:
            try:
                os.kill(self.processo.pid, signal.SIGKILL)  # Força o encerramento do processo
            except OSError:
                pass  # O processo já pode ter sido finalizado
            self.processo.wait()
            self.processo = None
    def start(self):

        if self.running == 0:
            self.finalizar_programa()
            self.processo = subprocess.Popen(["python", self.comando1])
            self.running = 1
        else:
            self.finalizar_programa()
            self.processo = subprocess.Popen(["python", self.comando0])
            self.running = 0

    def run(self):
        try:
            tk.Button(self.root,text='Trocar Camera',command= lambda:self.start()).pack(pady=60)
            self.root.mainloop()
        except Exception:
            self.finalizar_programa()

if __name__ == '__main__':
    app = IA()
    app.run()
    app.finalizar_programa()

