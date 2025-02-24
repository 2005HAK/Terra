from ultralytics import YOLO
model = YOLO('yolo11n.pt')
while True:
    for results in model.predict(source=2, show=True,save=False):
        for result in results:
            print(results)
