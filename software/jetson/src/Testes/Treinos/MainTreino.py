from TrainLibrary import set_epochs,set_modelo,treinar,val_metricas
from time import time,sleep
results_dict = {}
final_dict = {'best_map': 0, 'best_epochs': 0, 'best_loss': float('inf')}
dataset_path = "datasets/data.yaml"
modelo = set_modelo()
epochs_list = set_epochs()

for epochs in epochs_list:

    start_time,results = treinar(modelo,epochs,dataset_path)
    end_time = time()
    print(f"Treinamento de {epochs} epochs concluído em {time()-start_time:.2f} segundos.")
    sleep(3)
    results_dict,final_dict = val_metricas(results_dict,results,epochs,final_dict)

    #    mAP (Mean Average Precision)
    # Pega o valor de mAP e loss final ou "metrics/mAP_50-95"


print("\nResultados finais:")
for epochs, metrics in results_dict.items():
    print(f"{epochs} epochs -> mAP: {metrics['mAP']:.4f}, Val Loss: {metrics['val_loss']:.4f}")

print(f"\nMelhor quantidade de epochs: {final_dict['best_epochs']} com mAP de {final_dict['best_map']:.4f} e perda de {final_dict['best_loss']}")