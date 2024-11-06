# O programa ainda não está funcional

from ultralytics import YOLO
from time import time

epochs_list = []
qnt = int(input("Quantos modelos gostaria de treinar?\n"))
for i in range(qnt):
    epochs_list.append(int(input(f"Quantos Epochs para o treinamento {i+1}?\n")))

model_choice = int(input("Em qual modelo gostaria de treinar?\n[1][yolov8n.pt]\n[2][yolo11n.pt]\n"))
model_path = "modelos/yolov8n.pt" if model_choice == 1 else "modelos/yolo11n.pt"
model = YOLO(model_path)

best_epochs = None
best_map = 0
results_dict = {}

dataset_path = "C:/Users/Gustavo/Desktop/Terra/software/jetson/src/Testes/datasets/data.yaml"

for epochs in epochs_list:
    print(f"\nTreinando com {epochs} epochs...")
    start_time = time()
    results = model.train(
    data=dataset_path,
    epochs=epochs,
    imgsz=640,
    project="modelos/train/best_epochs_experiment",
    name=f"exp_epochs_{epochs}",
    save=True,
    verbose=True
    )

    print(f"\nTreinamento de {epochs} epochs concluído em {start_time-time():.2f} segundos.")

    # Extraindo métricas
    final_map = results.metrics.get("metrics/mAP_50", 0)  # Definindo valor padrão caso a métrica não exista
    final_val_loss = results.metrics.get("metrics/val/loss", 0)

    results_dict[epochs] = {
        "mAP": final_map,
        "val_loss": final_val_loss
    }

    # Comparação para encontrar o melhor modelo
    if final_map > best_map:
        best_map = final_map
        best_epochs = epochs

# Resultados finais
print("\nResultados finais:")
for epochs, metrics in results_dict.items():
    print(f"{epochs} epochs -> mAP: {metrics['mAP']:.4f}, Val Loss: {metrics['val_loss']:.4f}")

print(f"\nMelhor quantidade de epochs: {best_epochs} com mAP de {best_map:.4f}")
