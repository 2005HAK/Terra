from ultralytics import YOLO
from os import listdir
from time import time
def set_epochs():

    qnt = int(input("Quantos modelos gostaria de treinar?\n"))
    epochs_list = []
    for i in range(qnt):
        epochs_list.append(int(input(f"Quantos Epochs para o treinamento {i+1}?\n")))
    return epochs_list

def set_modelo():

    modelos = listdir('software/jetson/src/Testes/modelos')
    x=0
    print("Qual Versão Gostaria de Escolher Como Base?")
    for mod in modelos:
        print("[",x,"][",mod,"]")
        x+=1
    esc = int(input())
    return YOLO('modelos/'+modelos[esc])

def treinar(model, epochs, dataset_path):

    print(f"\nTreinando com {epochs} epochs...")
    start_time = time()
    results = model.train(
        data=dataset_path,
        epochs=epochs,
        imgsz=640,
        project="runs/detect/best_epochs_experiment",
        name=f"exp_epochs_{epochs}",
        save=True
    )
    return start_time,results


def val_metricas(results_dict, results, epochs, final_dict):

    results_dict[epochs] = {

        "mAP": results.metrics["metrics/mAP_50"],
        "val_loss": results.metrics["metrics/val/loss"]

    }
    if results.metrics["metrics/mAP_50"] > final_dict['best_map']:
        final_dict = {
        'best_map':  results.metrics['metrics'],
        'best_epochs':  epochs,
        'best_loss': results.metrics["metrics/val/loss"]
        }
    return results_dict,final_dict