from ultralytics import YOLO
model = YOLO("yolo11n.pt")
while True:
    for results in model.predict(source=0, verbose=False, stream=True, show=True,imgsz=160,save=False,max_det=1):
        for result in results:
            boxes = result.boxes.data.cpu().numpy().tolist()
            dicio = {'data': boxes, 'names': result.names}
            for datas in result[0].boxes.data:
                print(f"Nome: {result[0].names[int(datas[5])]}\nConfiança: {datas[4]*100}%")