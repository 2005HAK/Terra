from socket import socket, AF_INET, SOCK_STREAM
from json import loads, JSONDecodeError
from time import time

class Ia:
    def __init__(self):
        # Initialize UART connection on Raspberry Pi
        print("Object IA created")
        self.identified_objects = []
        self.last_time = None
        self.current_time = time()
        self.host = "0.0.0.0"
        self.port = 65432
        self.socket = socket(AF_INET, SOCK_STREAM)
        self.socket.bind((self.host, self.port))
        self.socket.listen()
        print("Socket opened")

    def update_data(self):
        while True:
            conn, addr = self.socket.accept()
            with conn:
                # print(f"Conectado a addr: {addr}")
                while True:
                    dct = conn.recv(2048)
                    if not dct:
                        break
                    try:
                        received_dict = loads(dct.decode("utf-8"))
                        data = received_dict["data"]
                        names = received_dict["names"]

                        self.identified_objects = []

                        for dat in data:
                            object_detected = [
                                names.get(str(int(dat[5])), "Desconhecido"),
                                [int(dat[0]), int(dat[1])],
                                [int(dat[2]), int(dat[3])],
                                dat[4],
                            ]
                            self.identified_objects.append(object_detected)
                        
                        self.last_time = self.current_time
                        self.current_time = time()

                        # print(self.identified_objects)

                    except JSONDecodeError:
                        print("Erro ao decodificar JSON.")
                    except Exception as e:
                        print(f"Ocorreu um erro: {e}")

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
                required_object = []
                # required_object = [obj[1][0], obj[1][1], obj[2][0], obj[2][1]] # testar
                required_object.extend(obj[1])
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
                if obj[3] > confidence_object[3]:
                    confidence_object = obj[:]

        return confidence_object[0] if confidence_object is not None else None
    
    def stop(self):
        self.socket.close()
        print("Socket closed")
    
    def get_del_time(self):
        return self.current_time - self.last_time
