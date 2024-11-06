from ultralytics import YOLO
from os import listdir
from time import time


def set_epochs():
    try:
        qnt = int(input("Quantos modelos gostaria de treinar?\n"))
        epochs_list = []
        for i in range(qnt):
            epochs_list.append(int(input(f"Quantos Epochs para o treinamento {i + 1}?\n")))
        return epochs_list
    except ValueError:
        print("Por favor, insira um número inteiro válido.")
        return set_epochs()


def set_modelo():
    try:
        modelos = listdir('modelos')
        print("Qual Versão Gostaria de Escolher Como Base?")
        for idx, mod in enumerate(modelos):
            print(f"[{idx}] {mod}")

        esc = int(input("Digite o número do modelo desejado:\n"))
        return YOLO(f'modelos/{modelos[esc]}')
    except (ValueError, IndexError):
        print("Seleção inválida. Por favor, escolha um número de modelo válido.")
        return set_modelo()


def treinar(model, epochs, dataset_path):
    print(f"\nTreinando com {epochs} epochs...")
    start_time = time()
    results = model.train(
        data=dataset_path,
        epochs=epochs,
        imgsz=640,
        project="runs/detect/best_epochs_experiment",
        save=True
    )
    return start_time, results


def val_metricas(results_dict, results, epochs, final_dict):
    try:
        # Captura e armazena métricas
        mAP = results.metrics["metrics/mAP_50"]
        val_loss = results.metrics["metrics/val/loss"]

        results_dict[epochs] = {
            "mAP": mAP,
            "val_loss": val_loss
        }

        # Verifica se este é o melhor resultado até agora
        if mAP > final_dict['best_map']:
            final_dict['best_map'] = mAP
            final_dict['best_epochs'] = epochs
            final_dict['best_loss'] = val_loss

    except KeyError as e:
        print(f"Erro ao acessar métricas: {e}")

    return results_dict, final_dict
