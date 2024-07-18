IMAGE_WIDTH = 1280
IMAGE_HEIGHT = 720

IMAGE_CENTER = [IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2]

ERROR_CENTER = 50

SAFE_SECURITY = 1

from pymavlink import mavutil
import pixhawk as px
import math as m

def extract_boxes(data_received):
    """
    Removes the coordinates of the upper left and lower right points of the object from the data sent by Jetson

    :param data_received: Jetson data with object coordinates

    :return: sends the coordinates x0, y0, x1, y1 as a list 
    """

    return data_received["boxes"]

def center(xyxy = []):
    """
    Calculates the centers of the object

    :param: x and y coordinates of the detected object sent as a list to the function

    :return: x and y coordinates as a list of center or [-1, -1] if param is null
    """

    return [(xyxy[0] + xyxy[2]) / 2, (xyxy[1] + xyxy[3]) / 2] if xyxy else [-1, -1]

def set_power(bounding_box = [], distance = []):
    """
    Defines the power that motors execution the moviment

    Uses the diference into the object midpoint and image midpoint to calculate a value of power. The difference is multiplicate for weigth horizontal and vertical to return the value of power in each directions

    :param xyxy: x and y coordinates of the detected object sent as a list to the function

    :return: A list with power_h, horizontal power, and power_v, vertical power, from 0-100%
    """

    # Defines the power maximum that motors can receive (in %)
    POWER_MAX = 45

    powers = []

    if(bounding_box and not distance):
        power_v = 0
        power_h = 0

        k_p_x = 0.5
        k_p_y = 0.5

        xm, ym = center(bounding_box)

        error_x = xm - IMAGE_CENTER[0]
        error_y = ym - IMAGE_CENTER[1]

        power_h = k_p_x * m.fabs(error_x)
        power_v = k_p_y * m.fabs(error_y)

        power_h = max(min(power_h, POWER_MAX), 0)
        power_v = max(min(power_v, POWER_MAX), 0)

        powers.append(power_h)
        powers.append(power_v)
    elif(distance and not bounding_box):
        power_f = 0

        k_p_f = 4.5

        error_f = distance - SAFE_SECURITY

        power_f = k_p_f * m.fabs(error_f)

        power_f = max(min(power_f, POWER_MAX), 0)

        powers.append(power_f)

    return powers

def center_object(xyxy):
    """
    Decides which movement to take based on the position of the object in the image.

    :param xyxy: x and y coordinates of the detected object sent a list to the function

    :return: movement that the AUV must take
    """

    dir_h = ""
    dir_v = ""
    power_h = 0
    power_v = 0

    xm, ym = center(xyxy)

    if(xm >= 0 and xm <= IMAGE_WIDTH and ym >= 0 and ym <= IMAGE_HEIGHT):
        if(xm < IMAGE_CENTER[0] - (ERROR_CENTER / 2)):
            dir_h = "LEFT"
        elif(xm > IMAGE_CENTER[0] + (ERROR_CENTER / 2)):
            dir_h = "RIGHT"
        if(ym < IMAGE_CENTER[1] - (ERROR_CENTER / 2)):
            dir_v = "UP"
        elif(ym > IMAGE_CENTER[1] + (ERROR_CENTER / 2)):
            dir_v = "DOWN"

    power_h, power_v = set_power(bounding_box = xyxy)

    return [dir_h, power_h, dir_v, power_v]

def calculate_distance(object_class, xyxy):
    """
    Calculates the distance between AUV and object based on the object's actual width and image dimension

    :param object_class: The class of the detected object
    :param xyxy: Coordinates of the bounding box of the detected object

    :return: The distance between AUV and object in meters
    """

    # Actual width of the objects (in meters)
    width_objects = {"obj1": 2, "obj2": 1.5}

    # Initializes the variable with invalid value to indicates error
    distance_object = -1

    if (object_class in width_objects) and (xyxy[2] - xyxy[0] != 0):
        # Image diagonal (in pixels)
        d = m.sqrt(m.pow(IMAGE_WIDTH, 2) + m.pow(IMAGE_HEIGHT, 2))

        # Diagonal field of view (in rad)
        a = (m.pi / 180) * 55

        # focal distance
        f = (d / 2) * (m.cos(a / 2) / m.sin(a / 2))

        distance_object = (f * width_objects[object_class]) / (xyxy[2] - xyxy[0])

    return distance_object

# not tested
def advance(object_class, xyxy):
    """
    Decides whether to advance to the object and the power that will be used

    :param object_class: The class of the detected object
    :param xyxy: Coordinates of the bounding box of the detected object

    :return: action and power that must be used
    """
    
    distance_object = calculate_distance(object_class, xyxy)

    action = ""
    power = 0

    if(distance_object > SAFE_SECURITY):
        action = "FORWARD"

        power = set_power(distance = distance_object)
    
    return [action, power]

# tested in the simulator
def collision_detect(direction = False):
    """
    Detects whether the AUV has crashed based on acceleretion data from Pixhawk

    :param connection: Connection with Pixhawk
    :param direction: Whether acceleration data will be returned

    :return: If the AUV collided and the acceleration data if the "direction" variable is True
    """

    response = [False]
    ACC_LIMIT = 15
    sensors = px.get_data()
    acceleration = [sensors[0], sensors[1], sensors[2]]

    if any(m.fabs(acc) > ACC_LIMIT for acc in acceleration):
        response[0] = True

        if direction:
            response.append(acceleration)

    return response

# tests

# power = set_power(distance=1.5)
# print(power)

# import random

# def test(coord_inicial):
#     xyxy = coord_inicial

#     decisao = center_object(xyxy)

#     while(decisao[0] != "" or decisao[2] != ""):

#         if(decisao[0] == "LEFT"):
#             xyxy[0] = xyxy[0] + 1
#             xyxy[2] = xyxy[2] + 1
#         if(decisao[0] == "RIGHT"):
#             xyxy[0] = xyxy[0] - 1
#             xyxy[2] = xyxy[2] - 1
#         if(decisao[2] == "UP"):
#             xyxy[1] = xyxy[1] + 1
#             xyxy[3] = xyxy[3] + 1
#         if(decisao[2] == "DOWN"):
#             xyxy[1] = xyxy[1] - 1
#             xyxy[3] = xyxy[3] - 1
        
#         decisao = center_object(xyxy)
        
#         print(decisao)
#         print(xyxy)

# test([random.randint(0, 1280), random.randint(0, 720), random.randint(0, 1280), random.randint(0, 720)])

# xyxy = extract_boxes({"boxes": [0,0,3,4]})

# print(center_object(xyxy))

while True:
    print(collision_detect(direction=True))