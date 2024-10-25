from ultralytics import YOLO
from ultralytics.utils.checks import cuda_is_available
print("CUDA" if cuda_is_available() else "CPU")
model = YOLO("runs/detect/train3/weights/best.pt")
results = model('Video.mp4',stream=False,show=True,half=True,save=False)
i = 0
mediac = 0
for result in results:
    for datas in result.boxes.data:
        i+=1
        mediac+=datas[4]
        print(f"Confiança: {datas[4]*100:.4}%")
print(f"A Media de confiança é: {(mediac/i)*100:.4}")
print(f"Quantidade de detecções: {i}")