import serial
import json
import time

class Ia:
    def __init__(self):
        # Initialize UART connection on Raspberry Pi
        self.connection = serial.Serial('/dev/serial0', baudrate=115200, timeout=1)
        time.sleep(1)
        self.identified_objects = []
        
    def update_data(self):
        if self.connection.in_waiting > 0:
            # Read the serialized data from UART
            received_data = self.connection.readline().decode('utf-8')

            # Deserialize the JSON data
            try:
                parsed_data = json.loads(received_data)
                
                # Access the bounding box coordinates and class IDs
                data = parsed_data["datas"]
                # Names precisa mesmo ser passado toda vez? não pode ser somente na primeira e manter salvo na rasp?
                names = parsed_data["names"]
                
                self.identified_objects = []

                for dat in data:
                    object_detected = [names[int(dat[5])], [int(dat[0]), int(dat[1])], [int(dat[2]), int(dat[3])], dat[4]]
                    self.identified_objects.append(object_detected)

                self.connection.write("ACK\n".encode('utf-8'))
            except json.JSONDecodeError as e:
                print("Error decoding JSON:", e)

    def found_object(self):
        """
        Returns whether objects were found ou not

        :return: True if it identified objects and false if not identify
        """

        return False if self.identified_objects == [] else True

    def get_xyxy(self, object_name):
        """
        Returns the required object if it is found in the list of identified objects

        :param object_name: Name of the required object

        :return required_object: Baunding box coordinates of the required object
        """

        required_object = None

        for obj in self.identified_objects:
            if obj[0] == object_name:
                required_object = [obj[1]]
                required_object.extend(obj[2])
        
        return required_object
    
    def greater_confidence_object(self):
        """
        Identifies the object with greater confidence

        :return: Returns the object name with highest confidence or None if there are no objects
        """

        confidence_object = None

        if self.found_object():
            confidence_object = self.identified_objects[0]

            for obj in self.identified_objects:
                if obj[5] > confidence_object[5]:
                    confidence_object = obj[:]

        return confidence_object[0] if confidence_object is not None else None