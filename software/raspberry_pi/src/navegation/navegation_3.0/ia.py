import serial
import json
import time

class Ia:
    def __init__(self):
        # Initialize UART connection on Raspberry Pi
        self.connection = serial.Serial('/dev/serial0', baudrate=115200, timeout=1)
        time.sleep(1)
        self.data = None
        self.class_names = None
        
    def update_data(self):
        if self.connection.in_waiting > 0:
            # Read the serialized data from UART
            received_data = self.connection.readline().decode('utf-8')

            # Deserialize the JSON data
            try:
                parsed_data = json.loads(received_data)
                
                # Access the bounding box coordinates and class IDs
                datas = parsed_data["datas"]
                names = parsed_data["names"]

                # Process the information as needed
                for dat in datas:
                    print(f"Objeto: {names[int(dat[5])]}")
                    print(f"Cordenada X min: {dat[0]}")
                    print(f"Cordenada Y min: {dat[1]}")
                    print(f"Cordenada X max: {dat[2]}")
                    print(f"Cordenada Y min: {dat[3]}")
                    print(f"Certeza: {(dat[4]*100):.2}%")


                self.connection.write("ACK\n", encode('utf-8'))
            except json.JSONDecodeError as e:
                print("Error decoding JSON:", e)

    def get_data(self):
        return self.data
    
    def get_class_names(self):
        return self.class_names