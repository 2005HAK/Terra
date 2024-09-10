from ultralytics import YOLO
import cv2
import serial
import json
import time

# Initialize UART connection
ser = serial.Serial('/dev/ttyTHS1', baudrate=115200, timeout=1)
time.sleep(1)

# Load a model
model = YOLO('yolov8n.pt')  # pretrained YOLOv8n model

# Starts the webcam
webcam = cv2.VideoCapture(0)

while True:
    if webcam.isOpened():
        ret, frame = webcam.read()  # reads a single frame from the webcam
        results = model(source=frame, stream=True)  # return a generator of Results objects

        # Process results generator
        for result in results:
            boxes = result.boxes  # Boxes object for bbox outputs

            # Assuming there is at least one detected object
            if len(boxes) > 0:
                # Serialize data
                xyxylist = boxes.xyxy.cpu().numpy().tolist() if boxes.xyxy is not None else []
                class_ids_list = boxes.id.cpu().numpy.tolist() if boxes.id is not None else []
                data_to_send = {"boxes": xyxylist, "class_ids": class_ids_list}
                serialized_data = json.dumps(data_to_send)

                # Send serialized data over UART
                ser.write(serialized_data.encode())
                ser.write("\n".encode('utf-8'))

                while True:
                    if ser.in_waiting > 0:
                            confirmation = ser.readline().decode('utf-8').strip()
                            if confirmation == "ACK":
                                print("Confirmed")
                                break
