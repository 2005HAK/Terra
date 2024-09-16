import serial
import json
import time

# Initialize UART connection on Raspberry Pi
ser = serial.Serial('/dev/serial0', baudrate=115200, timeout=1)
time.sleep(1)

while True:
    if ser.in_waiting > 0:
        # Read the serialized data from UART
        received_data = ser.readline().decode('utf-8')

        # Deserialize the JSON data
        try:
            parsed_data = json.loads(received_data)
            
            # Access the bounding box coordinates and class IDs
            boxes = parsed_data["boxes"]
            class_ids = parsed_data["class_ids"]

            # Process the information as needed
            print("Received Data - Boxes:", boxes, "Class IDs:", class_ids)

            ser.write("ACK\n",encode('utf-8'))
        except json.JSONDecodeError as e:
            print("Error decoding JSON:", e)
