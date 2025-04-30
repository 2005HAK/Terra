import os
import torch
import torch.distributed as dist
from torch.nn.parallel import DistributedDataParallel as DDP
from ultralytics import YOLO  # Requer YOLOv8 instalado
from ultralytics.cfg import get_cfg

def main():
    # Inicializa o processo distribuído
    dist.init_process_group(backend='nccl')
    local_rank = int(os.environ["LOCAL_RANK"])
    torch.cuda.set_device(local_rank)

    # Configurações do treinamento
    cfg = get_cfg(overrides={
        'model': 'yolo11n.pt',  
        'data': 'data.yaml',      
        'epochs': 50,
        'batch': 16,
        'imgsz': 640,
        'device': local_rank
    })

    # Cria o modelo
    model = YOLO(cfg['model'])

    # Move o modelo para a GPU local
    model.model.to(local_rank)

    # Envolve com DDP
    model.model = DDP(model.model, device_ids=[local_rank])

    # Começa o treino
    model.train(cfg)

    # Finaliza o processo distribuído
    dist.destroy_process_group()

if __name__ == '__main__':
    main()
