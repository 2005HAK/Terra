IMAGE_WIDTH = 1280
IMAGE_HEIGHT = 720

IMAGE_CENTER = [IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2]

ERROR_CENTER = 50

import math as m

def extract_boxes(data_received):
    """
    Removes the coordinates of the upper left and lower right points of the object from the data sent by Jetson

    :param data_received: Jetson data with object coordinates

    :return: sends the coordinates x0, y0, x1, y1 as a list 
    """

    return data_received["boxes"]

def CENTER(xyxy = []):
    """
    Calculates the CENTERs of the object

    :param: x and y coordinates of the detected object sent as a list to the function

    :return: x and y coordinates as a list of CENTER or [-1, -1] if param is null
    """

    return [(xyxy[0] + xyxy[2]) / 2, (xyxy[1] + xyxy[3]) / 2] if xyxy else [-1, -1]

def center_object(xyxy):
    """
    Decides which movement to take based on the position of the object in the image.

    :param xyxy: x and y coordinates of the detected object sent a list to the function

    :return: movement that the AUV must take
    """

    dir1 = ""
    dir2 = ""

    xm, ym = CENTER(xyxy)

    if(xm >= 0 and xm <= IMAGE_WIDTH and ym >= 0 and ym <= IMAGE_HEIGHT):
        if(xm < IMAGE_CENTER[0] - (ERROR_CENTER / 2)):
            dir1 = "LEFT"
        elif(xm > IMAGE_CENTER[0] + (ERROR_CENTER / 2)):
            dir1 = "RIGHT"
        if(ym < IMAGE_CENTER[1] - (ERROR_CENTER / 2)):
            dir2 = "UP"
        elif(ym > IMAGE_CENTER[1] + (ERROR_CENTER / 2)):
            dir2 = "DOWN"

    power = set_power(xyxy)

    return [dir1, power[0], dir2, power[1]]

def set_power(xyxy):
    power_v = 0
    power_h = 0

    k_p_x = 0.5
    k_p_y = 0.5

    xm, ym = CENTER(xyxy)

    error_x = xm - IMAGE_CENTER[0]
    error_y = ym - IMAGE_CENTER[1]

    power_h = k_p_x * m.fabs(error_x)
    power_v = k_p_y * m.fabs(error_y)

    power_h = max(min(power_h, 45), 0)
    power_v = max(min(power_v, 45), 0)

    return [power_h, power_v]

import random

def test(coord_inicial):
    xyxy = coord_inicial

    decisao = center_object(xyxy)

    while(decisao[0] != "" or decisao[2] != ""):

        if(decisao[0] == "LEFT"):
            xyxy[0] = xyxy[0] + 1
            xyxy[2] = xyxy[2] + 1
        if(decisao[0] == "RIGHT"):
            xyxy[0] = xyxy[0] - 1
            xyxy[2] = xyxy[2] - 1
        if(decisao[2] == "UP"):
            xyxy[1] = xyxy[1] + 1
            xyxy[3] = xyxy[3] + 1
        if(decisao[2] == "DOWN"):
            xyxy[1] = xyxy[1] - 1
            xyxy[3] = xyxy[3] - 1
        
        decisao = center_object(xyxy)
        
        print(decisao)
        print(xyxy)

test([random.randint(0, 1280), random.randint(0, 720), random.randint(0, 1280), random.randint(0, 720)])

# xyxy = extract_boxes({"boxes": [0,0,3,4]})

# print(center_object(xyxy))