import torch
import cv2
import numpy as np


def load_yolov11_model(model_path):
    # Carrega o modelo YOLOv11
    checkpoint = torch.load(model_path, map_location='cpu')
    model = checkpoint['model'].float()
    model.eval()
    return model


def preprocess(image, img_size=640):
    # Pré-processamento para YOLOv11
    img = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    img = cv2.resize(img, (img_size, img_size))
    img = img.transpose(2, 0, 1)  # HWC to CHW
    img = np.ascontiguousarray(img)
    img = torch.from_numpy(img).float()
    img /= 255.0  # Normaliza 0-255 para 0.0-1.0
    return img.unsqueeze(0)  # Adiciona dimensão do batch


def postprocess(prediction, original_shape, conf_threshold=0.25):
    boxes = []
    # Pega apenas a primeira saída (tensor [1, 6, 8400])
    if isinstance(prediction, (list, tuple)):
        prediction = prediction[0]

    # Converte para numpy e remodela
    pred = prediction[0].cpu().numpy()  # [6, 8400]
    pred = pred.transpose()  # [8400, 6]

    # Filtra por confiança
    mask = pred[:, 4] > conf_threshold
    detections = pred[mask]

    # Processa cada detecção
    for det in detections:
        *xyxy, conf, cls = det
        # Ajusta coordenadas para o tamanho original
        h, w = original_shape[:2]
        x1, y1, x2, y2 = [int(coord * w / 640) for coord in xyxy]
        boxes.append([x1, y1, x2, y2, float(conf), int(cls)])

    return boxes


def main():
    model_path = "../modelos/SHARK_SAWFISH.pt"
    class_names = ['SAWFISH', 'SHARK']  # Substitua pelos nomes reais

    try:
        print("Carregando modelo YOLOv11...")
        model = load_yolov11_model(model_path)

        # Configura dispositivo
        device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        model.to(device)
        print(f"Dispositivo: {device}")

        # Configura webcam
        cap = cv2.VideoCapture(0)
        if not cap.isOpened():
            raise RuntimeError("Não foi possível abrir a webcam")

        print("Iniciando detecção (Pressione 'q' para sair)...")
        while True:
            ret, frame = cap.read()
            if not ret:
                print("Erro ao capturar frame da webcam")
                break

            try:
                # Pré-processamento
                img = preprocess(frame)
                img = img.to(device)

                # Inferência
                with torch.no_grad():
                    pred = model(img)

                # Pós-processamento
                boxes = postprocess(pred, frame.shape)

                # Desenha resultados
                for (x1, y1, x2, y2, conf, cls) in boxes:
                    cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                    label = f"{class_names[cls]}: {conf:.2f}"
                    cv2.putText(frame, label, (x1, y1 - 10),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

                cv2.imshow("YOLOv11 Detection", frame)
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break

            except Exception as e:
                print(f"Erro durante detecção: {str(e)}")
                break

    except Exception as e:
        print(f"Erro principal: {str(e)}")
    finally:
        if 'cap' in locals():
            cap.release()
        cv2.destroyAllWindows()
        print("Recursos liberados")


if __name__ == "__main__":
    main()