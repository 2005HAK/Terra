def extract_boxes(data_received):
    """
    Removes the coordinates of the upper left and lower right points of the object from the data sent by Jetson

    :param data_received: Jetson data with object coordinates

    :return: sends the coordinates x0, y0, x1, y1 as a list 
    """

    return data_received["boxes"]

def midpoint(xyxy = []):
    """
    Calculates the midpoints of the object

    :param: x and y coordinates of the detected object sent as a list to the function

    :return: x and y coordinates as a list of midpoint or [-1, -1] if param is null
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

    xm, ym = midpoint(xyxy)

    if(xm >= 0 and xm <= 1280 and ym >= 0 and ym <= 720):
        if(xm < 615):
            dir1 = "LEFT"
        elif(xm > 665):
            dir1 = "RIGHT"
        if(ym < 335):
            dir2 = "UP"
        elif(ym > 385):
            dir2 = "DOWN"

    return [dir1, dir2]

def set_power(xyxy):
    dx = xyxy[2] - xyxy[0]
    dy = xyxy[3] - xyxy[1]

import random

def test(coord_inicial):
    xyxy = coord_inicial

    decisao = center_object(xyxy)

    while(decisao[0] != "" or decisao[1] != ""):

        if(decisao[0] == "LEFT"):
            xyxy[0] = xyxy[0] + 1
            xyxy[2] = xyxy[2] + 1
        if(decisao[0] == "RIGHT"):
            xyxy[0] = xyxy[0] - 1
            xyxy[2] = xyxy[2] - 1
        if(decisao[1] == "UP"):
            xyxy[1] = xyxy[1] + 1
            xyxy[3] = xyxy[3] + 1
        if(decisao[1] == "DOWN"):
            xyxy[1] = xyxy[1] - 1
            xyxy[3] = xyxy[3] - 1
        
        decisao = center_object(xyxy)
        
        print(decisao)
        print(xyxy)

test([random.randint(0, 1280), random.randint(0, 720), random.randint(0, 1280), random.randint(0, 720)])

# xyxy = extract_boxes({"boxes": [0,0,3,4]})

# print(center_object(xyxy))