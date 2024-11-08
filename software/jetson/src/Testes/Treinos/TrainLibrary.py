from torch.mps import empty_cache
from ultralytics import YOLO
from os import listdir
from torch.cuda import empty_cache
from gc import collect
from multiprocessing import set_start_method
#Pede a quantidade de epochs ao usuario
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

#Pede ao usuario o modelo base de treino
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

#Treina o modelo com a quantidade de epochs requisitada (Funcional mas ainda Bugado)
def treinar(model, epochs, dataset_path):
        print(f"\nTreinando com {epochs} epochs...")
        set_start_method('spawn', force=True)
        try:
            results = model.train(
                device='cuda',
                batch=32,
                model=model,
                data=dataset_path,
                epochs=epochs,
                imgsz=640,
                name=f"Treino_Com_{epochs}_Epochs",
                project="TreinosCubo",
                save=True
            )
            empty_cache()
            collect()
            if results is None:
                print("Aviso: `model.train()` retornou None")
                return None
            else:
                return results

        except Exception as e:
            print(f"Erro durante o treinamento: {e}")
            return None

#Valida as metricas recebidas pelo treino (Ainda não testado)
def val_metricas(results_dict, results, epochs, final_dict):
    try:
        # Verifica se as métricas estão disponíveis
        if hasattr(results, 'maps') and "50" in results.maps:
            mAP = results.maps["50"]
        else:
            mAP = 0  # Valor padrão caso a métrica não esteja disponível

        if hasattr(results, 'results_dict') and "metrics/val/loss" in results.results_dict:
            val_loss = results.results_dict["metrics/val/loss"]
        else:
            val_loss = 0  # Valor padrão caso a métrica não esteja disponível

        # Armazena os resultados no dicionário de métricas
        results_dict[epochs] = {
            "mAP": mAP,
            "val_loss": val_loss
        }

        # Atualiza o melhor resultado até agora
        if mAP > final_dict['best_map']:
            final_dict['best_map'] = mAP
            final_dict['best_epochs'] = epochs
            final_dict['best_loss'] = val_loss

    except Exception as e:
        print(f"Erro ao processar métricas: {e}")

    return results_dict, final_dict
