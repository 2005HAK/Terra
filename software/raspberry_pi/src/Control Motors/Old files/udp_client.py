import base64
import json
import logging
import random
import re
import socket
import time
from io import BytesIO
from threading import Thread
import cv2
import numpy as np
from PIL import Image

from detect_gate import detect_gate
from detect_objects import detect_object, plot_detection_results
from follow_marker import detect_marker
from auv_control import align_buoy, align_marker, checkCenter

logger = logging.getLogger(__name__)

def replace_float_notation(string):
    """
    Replace unity float notation for languages like
    French or German that use comma instead of dot.
    This convert the json sent by Unity to a valid one.
    Ex: "test": 1,2, "key": 2 -> "test": 1.2, "key": 2

    :param string: (str) The incorrect json string
    :return: (str) Valid JSON string
    """
    regex_french_notation = r':(?P<num>[0-9,E-]+),'
    regex_end = r':(?P<num>[0-9,E-]+)}'

    for regex in [regex_french_notation, regex_end]:
        matches = re.finditer(regex, string, re.MULTILINE)

        for match in matches:
            num = match.group('num').replace(',', '.')
            string = string.replace(match.group('num'), num)
    return string


class SDUdpClient:
    def __init__(self, host, port):
        self.host = host
        self.port = port

        self.do_process_msgs = True

        self.socket = None
        self.th = None

        self.connect()

    def connect(self):
        self.socket = socket.socket(
            socket.AF_INET, socket.SOCK_DGRAM)

        # self.socket.connect((self.host, self.port))
        self.socket.bind(("0.0.0.0", self.port))

        # str_to_send = "test".encode("utf-8")
        # self.socket.sendto(str_to_send, (self.host, self.port))

        self.th = Thread(target=self.proc_msg)
        self.th.start()

    def proc_msg(self):
        # self.socket.setblocking(1)
        localbuffer = ""

        while self.do_process_msgs:

            bytesAddressPair = self.socket.recvfrom(1024*1024)
            data = bytesAddressPair[0]

            # we don't technically need to convert from bytes to string
            # for json.loads, but we do need a string in order to do
            # the split by \n newline char. This seperates each json msg.
            data = data.decode("utf-8")
            localbuffer += data

            try:
                n0 = localbuffer.find("{")
                n1 = localbuffer.rfind("}\n")
                if n1 >= 0 and 0 <= n0 < n1:  # there is at least one message :
                    msgs = localbuffer[n0: n1 + 1].split("\n")
                    localbuffer = localbuffer[n1:]

                    for m in msgs:
                        if len(m) <= 2:
                            continue

                        m = replace_float_notation(m)
                        j = json.loads(m)
                        self.on_msg_recv(j)

            except Exception as e:
                # empty the buffer (most likely the cause of the miss-reading)
                localbuffer = ""
                print("Got Exception")
                print(str(e))
                continue

    def on_msg_recv(self, j):
        logger.debug("got:" + j["msg_type"])

class SimpleUdpClient(SDUdpClient):

    def __init__(self, address, verbose=True):
        super().__init__(*address)
        self.last_images = [None, None]
        self.verbose = verbose
        self.name = str(random.random())
        self.states = ["CoinFlip","PASS THE GATE","FOLLOW THE MARKER","TOUCH THE BUOY","FINAL STATE","DONE"] # estados existentes na missao
        self.current_action = "FWD" # acao que sera enviada ao auv relativa a seu deslocamento
        self.model = None # Usado para carregar o modelo de deep learning na memoria
        self.count = 0 # Usado para realizar controles ligados ao tempo de uma manobra
        self.status_current_task = 1 # status da tarefa atual
        self.current_state = self.states[0] # string que indica em qual estado a missao se encontra no momento

    def on_msg_recv(self, json_packet):
        
        if json_packet['msg_type'] == "telemetry":
            image = Image.open(
                BytesIO(base64.b64decode(json_packet["CameraSensor"])))
            self.last_images[1] = cv2.cvtColor(
                np.asarray(image), cv2.COLOR_RGB2BGR)

            image = Image.open(
                BytesIO(base64.b64decode(json_packet["CameraSensor_1"])))
            self.last_images[0] = cv2.cvtColor(
                np.asarray(image), cv2.COLOR_RGB2BGR)
            start=time.time()
            #State that the auv will turn in the gate direction
            if self.current_state == "CoinFlip":
                if self.status_current_task == 1:
                    self.count += 1
                    if self.count <= 20:
                        self.status_current_task = 1
                        self.current_action = "TURN RIGHT"
                    else:
                        self.current_state = self.states[1]
                        self.count = 0
            # State that the auv will pass through the gate
            elif self.current_state == "PASS THE GATE":
                if self.status_current_task < 3:
                    self.current_action,self.last_images[1],self.status_current_task = detect_gate(self.last_images[1],self.model,self.status_current_task)
                if self.status_current_task == 3:
                    self.count += 1
                    if self.count >= 100:
                        self.current_state = self.states[2]
                        self.status_current_task = 1
                        self.current_action = "STOP"
                        self.count = 0
            # State that the AUV will align itself regarding to the marker
            elif self.current_state == "FOLLOW THE MARKER":
                if self.status_current_task == 1: # Pointing the camera to the background pool
                    if self.count <= 35:
                        self.current_action = "PITCH DOWN"
                        self.count += 1
                    else:
                        self.count = 0
                        self.status_current_task = 2
                elif self.status_current_task == 2: # Aligning the auv in relation to the marker
                    self.last_images[1],angle = detect_marker(self.last_images[1])
                    if angle != None:
                        self.last_images[1] = cv2.putText(self.last_images[1],"Angle: " + str(round(angle,2)) + " degrees",(30,30),cv2.FONT_HERSHEY_COMPLEX,1,(255,0,0),2,cv2.LINE_AA)
                        if angle <= 88.0:
                            self.current_action = align_marker(angle)
                        else:
                            self.status_current_task = 3
                elif self.status_current_task == 3: # Pointing the camera to the horizon level again (without pitch)
                    if self.count <= 35:
                        self.current_action = "PITCH UP"
                        self.count += 1
                    else:
                        self.count = 0
                        self.current_action = "STOP"
                        self.status_current_task = 1
                        self.current_state = self.states[3]
            # State that the AUV will detect the chosen buoy and touch it
            elif self.current_state == "TOUCH THE BUOY":
                if self.status_current_task == 1: # Auv moving forward before starting the search
                    if self.count <= 80:
                        self.current_action = "FWD"
                        self.count += 1
                    else:
                        self.count = 0
                        self.status_current_task = 2
                elif self.status_current_task == 2: # The search by the desired object begins
                    coordinates = detect_object(self.last_images[1],self.model,"Badge",0.6)
                    self.last_images[1] = plot_detection_results(self.last_images[1],coordinates,"Badge")
                    if len(coordinates) > 0: # If the object was detected
                        height = self.last_images[1].shape[0]
                        width = self.last_images[1].shape[1]
                        center_x = int((width*coordinates[0]["box"][1]+width*coordinates[0]["box"][3])/2) # center in x of the detected object
                        center_y = int((height*coordinates[0]["box"][0]+height*coordinates[0]["box"][2])/2) # center in y of the detected object

                        self.last_images[1] = cv2.circle(self.last_images[1],[int(center_x),int(center_y)],5,(0,0,255),2)
                        self.current_action = align_buoy(center_x,center_y,15)

                        top_left_y = height*coordinates[0]["box"][0]
                        top_left_x = width*coordinates[0]["box"][1]
                        bottom_right_y = height*coordinates[0]["box"][2]
                        bottom_right_x = width*coordinates[0]["box"][3]

                        if abs(bottom_right_y-top_left_y)*abs(bottom_right_x-top_left_x) >= 25000 and checkCenter(center_x,center_y,15,15) == True:
                            self.status_current_task = 3
                    else: # In case the system couldn't detect any object
                        self.current_action = "FWD"
                elif self.status_current_task == 3: # AUV moving forward to touch the buoy
                    if self.count <= 110:
                        self.current_action = "FWD"
                        self.count += 1
                    elif self.count > 110 and self.count <= 180:
                        self.current_action = "FWD REDUCED"
                        self.count += 1                       
                    else:
                        self.count = 0
                        self.status_current_task = 1
                        self.current_state = self.states[4]
            # State that the AUV maneuvers and adjust itself to finalize the competition
            elif self.current_state == "FINAL STATE":
                if self.status_current_task == 1:
                    if self.count <= 60:
                        self.current_action = "BACK"
                        self.count += 1
                    else:
                        self.count = 0
                        self.status_current_task = 2
                elif self.status_current_task == 2:
                    if self.count <= 30:
                        self.current_action = "TURN LEFT"
                        self.count += 1
                    else:
                        self.count = 0
                        self.status_current_task = 3
                elif self.status_current_task == 3:
                    if self.count <= 110:
                        self.current_action = "UP"
                        self.count += 1
                    else:
                        self.count = 0
                        self.status_current_task = 1
                        self.current_action = "STOP"
                        self.current_state = self.states[5]
            # AUV state reached when the sub ended the competition                           
            elif self.current_state == "DONE":
                self.current_action = "STOP"
            
            print(f'Estado atual : {self.current_state}')
            print(f'Status da tarefa atual : {self.status_current_task}')
            print(f'Contador: {self.count}')

            # display the image
            #cv2.imshow("Under Camera", self.last_images[0])
            end=time.time()
            cv2.putText(self.last_images[1],"FPS: "+str(round(1/(end-start),2)),(1000,30),cv2.FONT_HERSHEY_COMPLEX,0.8,(0,0,255),2,cv2.LINE_AA)
            self.last_images[1]=cv2.resize(self.last_images[1],(1920,1080))
            cv2.namedWindow("Front Camera", cv2.WND_PROP_FULLSCREEN)
            cv2.setWindowProperty("Front Camera",cv2.WND_PROP_FULLSCREEN,cv2.WINDOW_FULLSCREEN)
            cv2.imshow("Front Camera", self.last_images[1])
            cv2.waitKey(1)

                
            #cv2.imwrite(f"./imagens_portal/{self.count}.bmp",self.last_images[1])
            #self.count += 1

            # don't have to, but to clean up the print, delete the image string.
            del json_packet["CameraSensor"]
            del json_packet["CameraSensor_1"]

            '''
            if self.verbose:
                print(json_packet)
            '''

if __name__ == "__main__":
    # test just the UDP
    udp_client = SimpleUdpClient(("127.0.0.1", 9094))