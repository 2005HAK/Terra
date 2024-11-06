# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

# Descomentar as partes referentes a pixhawk e alterar as transições de estados

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

from enum import Enum, auto
import math as m
# import pixhawk as px
import ia
import control_motors as cm
import threading
import time # importar somente time, é oque esta sendo usado
from AUVError import *

# Width and height of the image seen by the camera
IMAGE_WIDTH = 640
IMAGE_HEIGHT = 480

OBJECT_INITIALIZATION = "Cube"

# Center of the image seen by the camera
IMAGE_CENTER = [IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2]

# Size of the zone that is considered the center of the image
ERROR_CENTER = 50

# Distance considered safe for the AUV to approach
SAFE_DISTANCE = .1

class State(Enum):
    """
    **State machine states**
    """

    INIT = auto()
    SEARCH = auto()
    CENTERING = auto()
    ADVANCING = auto()
    PASS_GATE_WITH_STYLE = auto()
    STABILIZING = auto()
    STOP = auto()

class AUVStateMachine:
    """
    **State machine implementation**
    """

    def __init__(self):
        print("State machine creation...")
        # self.last_state = None # pode ser util
        self.state = State.INIT
        self.next_state = None
        # self.pixhawk = px.Pixhawk()
        self.ia = ia.Ia()
        self.target_object = None
        self.motors = None
        self.distance = None # passar o calculo e armazenamento de distancia para a pix

        # Update sensors data and detection data in parallel with the state machine
        # self.sensor_thread = threading.Thread(target=self.update_sensors, daemon=True)
        self.detection_thread = threading.Thread(target=self.ia.update_data, daemon=True)
        # self.sensor_thread.start()
        self.detection_thread.start()

    def transition_to(self, new_state):
        """
        Transition between states

        :param new_state: Next state of the state machine
        :type new_state: State
        """

        print(f"Transitioning from {self.state} to {new_state}")
        # self.last_state = self.state # pode ser util
        self.state = new_state

    # Não testado
    def update_sensors(self):
        """
        Updates sensors data every **0.3 ms**
        """

        while True:
            self.pixhawk.update_data()
            time.sleep(0.3)
        
    # Não testado
    def checks_errors(self):
        """
        Checks for errors every **0.3 ms**
        """
        
        while True:
            self.pixhawk.collision_detect()
            time.sleep(0.3)

    def run(self):
        """
        Initializes the state machine
        """

        try:
            # Checks for errors in parallel with the state machine
            # self.errors_thread = threading.Thread(target=self.checks_errors, daemon=True)
            # self.errors_thread.start()

            while self.state != State.STOP:
                if self.state == State.INIT:
                    self.init()
                elif self.state == State.SEARCH:
                    self.search()
                elif self.state == State.CENTERING:
                    self.centering()
                elif self.state == State.ADVANCING:
                    self.advancing()
                elif self.state == State.STABILIZING:
                    self.stabilizing()

            self.stop()
        except AUVError as e:
            self.error_handling(e)

    # ERRORS HANDLING 
    def error_handling(self, e):
        """
        Handle errrors to return to operation

        :param e: Error detected
        :type e: AUVError
        """

        if isinstance(e, CollisionDetected):
            if self.state == State.SEARCH:
                self.direction_correction(e.acceleration)

    def direction_correction(self, acceleration):
        """
        Corrects the direction of the AUV by turning it 180º in relation to the crash location

        :param acceleration: Acceleration detected at the time of the crash
        :type acceleration: Float array
        """

        print("Correcting direction...")

        position_collision = -acceleration

        # angle = (acos(x / sqrt(x^2 + y^2)) * pi / 180) in rad
        angle = (m.acos(position_collision[0] / m.sqrt(m.pow(position_collision[0], 2) + m.pow(position_collision[1], 2))) * m.pi) / 180

        rotation_angle = m.pi - angle
        
        # Turn rigth by default
        action = "TURN RIGTH"

        # y > 0
        if position_collision[1] > 0:
            action = "TURN LEFT"

        self.rotate(angle=rotation_angle, action=action)
        
        self.run()

    # DEFINITION OF STATES
    def init(self):
        """
        **This state initializes the motors**
        """
        print("Searching for launcher...")
        
        while self.target_object != OBJECT_INITIALIZATION:
            self.search_objects()

        print("Initializing...")

        self.motors = cm.Motors()
        # self.transition_to(State.SEARCH)
        self.transition_to(State.CENTERING)
    
    def search(self):
        """
        **This state defines the search procedure**
        """

        # 1/8 turns
        rotation_current = 0

        print("Searching...")

        while self.target_object == None:
            if rotation_current < 8:
                self.rotate()
                rotation_current += 1
            else:
                #  terminar a essa parte
                self.motors.define_action({"DOWN": 20})
                rotation_current = 0

            self.search_objects()

        # verificar qual objeto(os) encontrou e responder de acordo
        
        self.transition_to(State.CENTERING)

    def search_objects(self):
        """
        Checks if objects were found. Found saved in target_object
        """

        if self.ia.found_object():
            self.target_object = self.ia.greater_confidence_object()

    def rotate(self, angle = 0.785398, error_angle = 0.174533, action = "TURN LEFT"):
        gyro_current = self.pixhawk.get_gyro()
        gyro_old = None

        rotated = 0

        while m.abs(rotated) < angle - error_angle:
            self.motors.define_action({action: 20})

            gyro_old = gyro_current
            gyro_current = self.pixhawk.get_gyro()
            delta_time = self.pixhawk.current_time - self.pixhawk.old_time

            rotated += delta_time * (gyro_current[2] + gyro_old[2]) / 2

        self.motors.define_action({action: 0})
            
    def centering(self):
        """
        **This state defines the centralization procedure**
        """

        print("Centering...")

        self.transition_to(State.SEARCH)
        
        lost_object = 0

        if self.ia.found_object():
            is_center = 0
            
            while not is_center:
                # se o objeto deixar de ser identificado pela ia deve dar um break no while e em search tentar buscar o objeto novamente
                xyxy = self.ia.get_xyxy(self.target_object)

                if xyxy != None:
                    actions = center_object(xyxy, self.ia.get_del_time())

                    # Mudar de dicionario para array (é mais rápido)

                    # print(f"{actions[1]}: {actions[2]}, {actions[3]}: {actions[4]}")

                    self.motors.define_action({actions[1]: actions[2], actions[3]: actions[4]})

                    is_center = actions[0]
                else:
                    is_center = 1
                    lost_object = 1
                    print("Lost object!")
            
            if not lost_object:
                self.motors.define_action({"DOWN": 0})               
                # self.next_state = State.ADVANCING
                # self.transition_to(State.STABILIZING)
                self.transition_to(State.ADVANCING)

    
    def advancing(self):
        """
        **This state difines the advancement procedure**
        """

        print("Advancing...")

        self.transition_to(State.SEARCH)

        advance = True

        if self.ia.found_object():
            while advance:
                # se o objeto deixar de ser identificado pela ia deve dar um break no while e em search tentar buscar o objeto novamente
                xyxy = self.ia.get_xyxy(self.target_object)
                
                self.distance = calculate_distance(self.target_object, xyxy)
                action = advance_decision(self.distance, self.ia.get_del_time())

                self.motors.define_action({action[1]: action[2]})

                advance = action[0]
            
            # self.next_state = State.STOP
            # self.transition_to(State.STABILIZING)
            self.transition_to(State.STOP)

    def stabilizing(self):
        """
        **This state stabilizes the AUV**
        """

        print("Stabilizing...")

        is_stable = False

        while not is_stable:
            velocity = self.pixhawk.get_vel()

            actions = stabilizes(velocity)

            self.motors.define_action({actions[1]: actions[2]}) # x
            time.sleep(.5)
            self.motors.define_action({actions[3]: actions[4], actions[5]: actions[6]}) # y e z
            time.sleep(.5)

            is_stable = actions[0]

        self.transition_to(self.next_state if self.next_state != None else State.SEARCH)
        self.next_state = None

    def pass_gate_with_style(self):
        pass

    def stop(self):
        """
        **This state defines the stopping procedure**
        """

        print("Stoping...")

        self.ia.stop()

        self.motors.finish()
    # END DEFINITION OF STATES

def center(xyxy = None):
    """
    Calculates the centers of the object

    :param xyxy: x and y coordinates of the detected object
    :type xyxy: Array

    :return: x and y coordinates as a list of center or [-1, -1] if param is null
    """

    return [(xyxy[0] + xyxy[2]) / 2, (xyxy[1] + xyxy[3]) / 2] if xyxy is not None else [-1, -1]

def center_set_power(bounding_box = None, del_time = None):
    """
    Defines the constants to part proportional and integral of power controller of centering

    :param bounding_box: up-left and down-right point of bounding box
    :type bounding_box: array of list of float
    :param del_time: Time variation between last date and current date
    :type del_time: Float
    """

    k_p = [.2, .15]
    k_i = [.5, .5]

    xm, ym = center(bounding_box)

    errors = [(xm - IMAGE_CENTER[0]), (ym - IMAGE_CENTER[1])]

    return set_power(k_p, k_i, errors, del_time)

def advance_set_power(distance = None, del_time = None):
    """
    Defines the constants to part proportional and integral of power controller of advance

    :param distance: Distance between AUV and object
    :type distance: Float
    :param del_time: Time variation between last date and current date
    :type del_time: Float
    """

    k_p = [100]
    k_i = [100]

    errors = [(distance - SAFE_DISTANCE)]

    return set_power(k_p, k_i, errors, del_time)

def stabilizes_set_power(velocity = None, del_time = None):
    """
    Defines the constants to part proportional and integral of power controller of stabilize

    :param velocity: AUV velocity in each axis
    :type velocity: Array of float
    :param del_time: Time variation between last date and current date
    :type del_time: Float
    """

    k_p = [1.5, 1.5, 1.5]
    k_i = [1.5, 1.5, 1.5]

    return set_power(k_p, k_i, velocity, del_time)

def set_power(k_p = None, k_i = None, errors = None, del_time = None):
    """
    Defines the power that motors execution the moviment
    
    Multiplies the error by a constant to send the required power

    :param k_p: constants to the proportional controler
    :type k_p: Array of float
    :param errors: Errors values between detect value and target value
    :type errors: Array of float
    :param del_time: Time variation between last date and current date
    :type del_time: Float

    :return: A array of float with power values
    """

    values = [0 for i in range(len(errors))]

    for i in range(len(errors)):
        e = m.fabs(errors[i])
        values[i] = max(min(k_p[i] * e + k_i[i] * (e * del_time), 100), 0)
    
    return values

def center_object(xyxy, del_time):
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
        if(xm < IMAGE_CENTER[0] - (ERROR_CENTER / 2)):
            dir_h = "LEFT"
        elif(xm > IMAGE_CENTER[0] + (ERROR_CENTER / 2)):
            dir_h = "RIGHT"
        if(ym < IMAGE_CENTER[1] - (ERROR_CENTER / 2)):
            dir_v = "UP"
        elif(ym > IMAGE_CENTER[1] + (ERROR_CENTER / 2)):
            dir_v = "DOWN"

    power_h, power_v = center_set_power(xyxy, del_time)

    return [0 if dir_h != "" or dir_v != "" else 1, dir_h, power_h, dir_v, power_v]

def calculate_distance(object_class, xyxy):
    """
    Calculates the distance between AUV and object based on the object's actual width and image dimension

    :param object_class: The class of the detected object
    :type object_class: String
    :param xyxy: Coordinates of the bounding box of the detected object
    :type xyxy: Array

    :return: The distance between AUV and object in meters
    """

    # Actual width of the objects (in meters)
    width_objects = {"Cube": 0.055, "cell phone": .075}

    # Initializes the variable with invalid value to indicates error
    object_distance = -1

    if (object_class in width_objects) and (xyxy[2] - xyxy[0] != 0):
        # Image diagonal (in pixels)
        d = m.sqrt(m.pow(IMAGE_WIDTH, 2) + m.pow(IMAGE_HEIGHT, 2))

        # Diagonal field of view (in rad)
        a = (m.pi / 180) * 55

        # focal distance
        f = (d / 2) * (m.cos(a / 2) / m.sin(a / 2))

        object_distance = (f * width_objects[object_class]) / (xyxy[2] - xyxy[0])
    
        print(f"Object_distance {object_distance:.4} m")

    return object_distance

def advance_decision(object_distance, del_time):
    """
    Decides whether to advance to the object and the power that will be used

    :param object_distance: Distance between the AUV and the object
    :type object_distance: Int

    :return: Whether advance or no, action and power that must be used
    """
    
    action = None
    power = 0

    if(object_distance > SAFE_DISTANCE):
        action = "FRONT"

        power = advance_set_power(object_distance, del_time)[0]
    
    return [1 if action != None else 0, action, power]

def stabilizes(velocity):
    """
    Defines the actions to stabilize the AUV

    :param velocity: Velocity values on the x, y and z axes, respectively
    :type velocity: Array

    :return: Whether it's stable or no and the moviments with their powers 
    """

    # Acceptable error in the velocity
    error_velocity = [.1, .1, .1]

    power_x = 0
    power_y = 0
    power_z = 0

    action_x = defines_action(velocity[0], error_velocity[0], "FRONT", "BACK")
    action_y = defines_action(velocity[1], error_velocity[1], "RIGHT", "LEFT")
    action_z = defines_action(velocity[2], error_velocity[2], "DOWN", "UP")

    is_stable = 1 if action_x == "" and action_y == "" and action_z == "" else 0

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