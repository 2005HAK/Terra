import ia
import threading

def search_objects():
	"""
	Checks if objects were found. Found saved in target_object
	"""

	if ia.found_object():
		target_object = ia.greater_confidence_object()

# Width and height of the image seen by the camera
IMAGE_WIDTH = 640
IMAGE_HEIGHT = 480

# Center of the image seen by the camera
IMAGE_CENTER = [IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2]

# Size of the zone that is considered the center of the image
ERROR_CENTER = 50

# Distance considered safe for the AUV to approach
SAFE_DISTANCE = 1

ia = ia.Ia()
target_object = None
distance = None

detection_thread = threading.Thread(target=ia.update_data, daemon=True)
detection_thread.start()

while target_object == None:
       print("Andando para frente")
       search_objects()

print(f"Target object is {target_object}")

print("Centering...")

xyxy = ia.get_xyxy(target_object)

is_center = False

while not is_center:
	# se o objeto deixar de ser identificado pela ia deve dar um break no while e em search tentar buscar o objeto novamente
	xyxy = ia.get_xyxy(target_object)
	
	print(xyxy)

        if xyxy != None:
		actions = center_object(xyxy)

		# Mudar de dicionario para array (é mais rápido)
            
        print(f"{actions[1]}: {actions[2]}, {actions[3]}: {actions[4]}")

		is_center = actions[0]
			
print("Advancing...")

advance = True

while advance:
	xyxy = ia.get_xyxy(self.target_object)
	while xyxy is None:
		xyxy = ia.get_xyxy(self.target_object)
	
	distance = calculate_distance(target_object, xyxy)
	action = advance(distance)

	motors.define_action({action[1]: action[2]})

	advance = action[0]

def set_power(bounding_box = None, distance = None, velocity = None):
    """
    Defines the power that motors execution the moviment

    Uses the diference into the object midpoint and image midpoint to calculate a value of power. The difference is multiplicate for weigth horizontal and vertical to return the value of power in each directions

    :param xyxy: x and y coordinates of the detected object
    :type xyxy: Array

    :return: A list with power_h, horizontal power, and power_v, vertical power, from 0-100%
    """

    # Defines the power maximum that motors can receive (in %)
    POWER_MAX = 45

    powers = []

    if bounding_box is not None and distance is None and velocity is None:
        power_v = 0
        power_h = 0

        k_p_h = 0.5
        k_p_v = 0.5

        xm, ym = center(bounding_box)

        error_x = xm - IMAGE_CENTER[0]
        error_y = ym - IMAGE_CENTER[1]

        power_h = k_p_h * m.fabs(error_x)
        power_v = k_p_v * m.fabs(error_y)

        power_h = max(min(power_h, POWER_MAX), 0)
        power_v = max(min(power_v, POWER_MAX), 0)

        powers.extend([power_h, power_v])

    elif distance is not None and bounding_box is None and velocity is None:
        power_f = 0

        k_p_f = 4.5

        error_f = distance - SAFE_DISTANCE

        power_f = k_p_f * m.fabs(error_f)

        power_f = max(min(power_f, POWER_MAX), 0)

        powers.append(power_f)
    
    elif velocity is not None and bounding_box is None and distance is None:
        k_p_x = 1.5
        k_p_y = 1.5
        k_p_z = 1.5

        power_x = k_p_x * m.fabs(velocity[0])
        power_y = k_p_y * m.fabs(velocity[1])
        power_z = k_p_z * m.fabs(velocity[2])

        power_x = max(min(power_x, POWER_MAX), 0)
        power_y = max(min(power_y, POWER_MAX), 0)
        power_z = max(min(power_z, POWER_MAX), 0)

        powers.extend([power_x, power_y, power_z])

    return powers

def center_object(xyxy):
    """
    Decides which movement to take based on the position of the object in the image.

    :param xyxy: x and y coordinates of the detected object
    :type xyxy: Array

    :return: Movement that the AUV must take
    """

    dir_h = ""
    dir_v = ""
    power_h = 0
    power_v = 0
    xm, ym = center(xyxy)

    if(xm >= 0 and xm <= IMAGE_WIDTH and ym >= 0 and ym <= IMAGE_HEIGHT):
        if(ym < IMAGE_CENTER[1] - (ERROR_CENTER / 2)):
            dir_v = "UP"
        elif(ym > IMAGE_CENTER[1] + (ERROR_CENTER / 2)):
            dir_v = "DOWN"

    power_h, power_v = set_power(bounding_box = xyxy)

    return [False if dir_h != "" or dir_v != "" else True, dir_h, power_h, dir_v, power_v]

def advance(object_distance):
    """
    Decides whether to advance to the object and the power that will be used

    :param object_distance: Distance between the AUV and the object
    :type object_distance: Int

    :return: Whether advance or no, action and power that must be used
    """
    
    action = None
    power = 0

    if(object_distance > SAFE_DISTANCE):
        action = "FORWARD"

        power = set_power(distance = object_distance)[0]
    
    return [True if action is not None else False, action, power]
