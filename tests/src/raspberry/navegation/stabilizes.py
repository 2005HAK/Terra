from enum import Enum, auto
import math as m
import pixhawk as px

IMAGE_WIDTH = 1280
IMAGE_HEIGHT = 720

IMAGE_CENTER = [IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2]

ERROR_CENTER = 50

SAFE_SECURITY = 1
def center():
    pass

def set_power(bounding_box = None, distance = None, velocity = None):
    """
    Defines the power that motors execution the moviment

    Uses the diference into the object midpoint and image midpoint to calculate a value of power. The difference is multiplicate for weigth horizontal and vertical to return the value of power in each directions

    :param xyxy: x and y coordinates of the detected object sent as a list to the function

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

        error_f = distance - SAFE_SECURITY

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

# def stabilizes(velocity):
#     """
#     Defines the actions to stabilize the AUV

#     :param velocity: List with velocity values on the x, y and z axes, respectively

#     :return: Whether it's stable or no and the moviments with their powers 
#     """

#     # Acceptable error in the velocity 
#     error_velocity = 0.1

#     action_x = ""
#     action_y = ""
#     action_z = ""

#     # Sets the action on x-axis
#     if velocity[0] > 0 + error_velocity:
#         action_x = "BACK"
#     elif velocity[0] < 0 - error_velocity:
#         action_x = "FRONT"

#     # Sets the action on y-axis
#     if velocity[1] > 0 + error_velocity:
#         action_y = "LEFT"
#     elif velocity[1] < 0 - error_velocity:
#         action_y = "RIGHT"

#     # Sets the action on z-axis
#     if velocity[2] > 0 + error_velocity:
#         action_z = "UP"
#     elif velocity[2] < 0 - error_velocity:
#         action_z = "DOWN"

#     power_x, power_y, power_z = set_power(velocity = velocity)

#     return [False if action_x != "" or action_y != "" or action_z != "" else True, action_x, power_x, action_y, power_y, action_z, power_z]

# def stabilizes(velocity):
#     """
#     Defines the actions to stabilize the AUV

#     :param velocity: List with velocity values on the x, y and z axes, respectively

#     :return: Whether it's stable or no and the moviments with their powers 
#     """

#     # Acceptable error in the velocity 
#     error_velocity = 0.1

#     is_stable = True

#     action_x = ""
#     action_y = ""
#     action_z = ""
#     power_x = 0
#     power_y = 0
#     power_z = 0

#     # Sets the action on x-axis
#     if velocity[0] > 0 + error_velocity:
#         action_x = "BACK"
#     elif velocity[0] < 0 - error_velocity:
#         action_x = "FRONT"

#     # Sets the action on y-axis
#     if velocity[1] > 0 + error_velocity:
#         action_y = "LEFT"
#     elif velocity[1] < 0 - error_velocity:
#         action_y = "RIGHT"

#     # Sets the action on z-axis
#     if velocity[2] > 0 + error_velocity:
#         action_z = "UP"
#     elif velocity[2] < 0 - error_velocity:
#         action_z = "DOWN"

#     if action_x != "" or action_y != "" or action_z != "":
#         power_x, power_y, power_z = set_power(velocity = velocity)

#         is_stable = False

#     return [is_stable, action_x, power_x, action_y, power_y, action_z, power_z]

def stabilizes(velocity):
    """
    Defines the actions to stabilize the AUV

    :param velocity: List with velocity values on the x, y and z axes, respectively

    :return: Whether it's stable or no and the moviments with their powers 
    """

    # Acceptable error in the velocity 
    error_velocity = 0.06

    power_x = 0
    power_y = 0
    power_z = 0

    action_x = defines_action(velocity[0], error_velocity, "FRONT", "BACK")
    action_y = defines_action(velocity[1], error_velocity, "RIGHT", "LEFT")
    action_z = defines_action(velocity[2], error_velocity, "DOWN", "UP")

    is_stable = action_x == "" and action_y == "" and action_z == ""

    if not is_stable:
        power_x, power_y, power_z = set_power(velocity = velocity)

    return [is_stable, action_x, power_x, action_y, power_y, action_z, power_z]

def defines_action(velocity, error_velocity, positive_action, negative_action):
    action = ""

    if velocity > 0 + error_velocity:
        action = negative_action
    elif velocity < 0 - error_velocity:
        action = positive_action
    
    return action

sensors = px.Pixhawk()

# Com filtro (estamos tendo problemas aqui)
# rx = 0.0006754251140591805
# ry = 0.0019773677451885305
# rz = 0.0008347233790354948
# ermx = .1
# ermy = .5
# ermz = .3
# eix = 0
# eiy = 0
# eiz = 0
# vex = .5
# vey = .5
# vez = .5
# vx = vex * vex + rx
# vy = vey * vey + ry
# vz = vez * vez + rz

# while True:
#     sensors.update_data()
#     # print(f"Velocidade: {sensors.get_vel()}")

#     gkx = vx / float(vx + ermx)
#     gky = vy / float(vy + ermy)
#     gkz = vz / float(vz + ermz)

#     estado_atualx = eix + gkx * (sensors.get_vel()[0] - eix)
#     estado_atualy = eiy + gky * (sensors.get_vel()[1] - eiy)
#     estado_atualz = eiz + gkz * (sensors.get_vel()[2] - eiz)

#     variacao_estado_atualx = (1 - gkx) * vx
#     variacao_estado_atualy = (1 - gky) * vy
#     variacao_estado_atualz = (1 - gkz) * vz

#     eix = estado_atualx
#     eiy = estado_atualy
#     eiz = estado_atualz

#     vx = variacao_estado_atualx + rx
#     vy = variacao_estado_atualy + ry
#     vz = variacao_estado_atualz + rz

#     # print([vx, vy, vz])

#     print(stabilizes([vx, vy, vz]))

# sem filtro
while True:
    sensors.update_data()
    # print(f"Velocidade: {sensors.get_vel()}")

    print(stabilizes(sensors.get_vel()))