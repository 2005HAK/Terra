from ultralytics import YOLO
from time import time
epochs_list = []
qnt = int(input("Quantos modelos gostaria de treinar?\n"))
for i in range(qnt):
    epochs_list.append(int(input(f"Quantos Epochs para o treinamento {i+1}?\n")))
if int(input("Em qual modelo gostaria de treinar?\n[1][yolov8n.pt]\n[2][yolo11n.pt]\n")) == 1:
    model = YOLO("modelos/yolov8n.pt")
else:
    model = YOLO("modelos/yolo11n.pt")
best_epochs = None
best_map = 0
results_dict = {}

dataset_path = "datasets/data.yaml"

for epochs in epochs_list:
    print(f"\nTreinando com {epochs} epochs...")
    start_time = time()

    for current_epoch in range(epochs):

        results = model.train(
            data=dataset_path,
            epochs=1,
            imgsz=640,
            project="runs/detect/best_epochs_experiment",
            name=f"exp_epochs_{epochs}",
            save=False
        )

        percent_complete = (current_epoch + 1) / epochs * 100
        print(f"Treinando em {epochs} Epochs: {percent_complete:.2f}% completo...", end="\r")

    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"Treinamento de {epochs} epochs concluído em {elapsed_time:.2f} segundos.")


    #    mAP (Mean Average Precision)
    # Pega o valor de mAP e loss final ou "metrics/mAP_50-95"


    final_map = results.metrics["metrics/mAP_50"]
    final_val_loss = results.metrics["metrics/val/loss"]

    results_dict[epochs] = {
        "mAP": final_map,
        "val_loss": final_val_loss
    }

    if final_map > best_map:
        best_map = final_map
        best_epochs = epochs

print("\nResultados finais:")
for epochs, metrics in results_dict.items():
    print(f"{epochs} epochs -> mAP: {metrics['mAP']:.4f}, Val Loss: {metrics['val_loss']:.4f}")

print(f"\nMelhor quantidade de epochs: {best_epochs} com mAP de {best_map:.4f}")