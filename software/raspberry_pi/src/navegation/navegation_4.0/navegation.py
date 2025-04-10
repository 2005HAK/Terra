from enum import Enum, auto
from math import acos, pi, sqrt, pow, cos, sin, fabs
from sensors import Sensors
from yoloctrl import YoloCtrl
from thrusters_control import Actions, ThrustersControl
from threading import Thread
from time import sleep
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
SAFE_DISTANCE = 1

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
        self.sensors = Sensors()
        self.yolo_ctrl = YoloCtrl()
        self.target_object = None
        self.thrusters = None
        self.distance = None # passar o calculo e armazenamento de distancia para a pix

        # Update sensors data and detection data in parallel with the state machine
        self.sensor_thread = Thread(target=self.sensors.update_data, daemon=True)
        self.detection_thread = Thread(target=self.yolo_ctrl.update_data, daemon=True)
        self.sensor_thread.start()
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
    def checks_errors(self):
        """
        Checks for errors every **0.3 ms**
        """
        
        while True:
            self.sensors.collision_detect()
            self.sensors.detect_overheat()
            sleep(0.1)

    def run(self):
        """
        Initializes the state machine
        """

        try:
            # Checks for errors in parallel with the state machine
            self.errors_thread = Thread(target=self.checks_errors, daemon=True)
            self.errors_thread.start()

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
        if isinstance(e, ImpossibleConnectThrusters) or isinstance(e, HighTempError):
            exit(1)

    def direction_correction(self, acceleration):
        """
        Corrects the direction of the AUV by turning it 180º in relation to the crash location

        :param acceleration: Acceleration detected at the time of the crash
        :type acceleration: Float array
        """

        print("Correcting direction...")

        position_collision = -acceleration

        # angle = (acos(x / sqrt(x^2 + y^2)) * pi / 180) in rad
        angle = (acos(position_collision[0] / sqrt(pow(position_collision[0], 2) + pow(position_collision[1], 2))) * pi) / 180

        rotation_angle = pi - angle
        
        # Turn rigth by default
        action = Actions.TURNRIGHT

        # y > 0
        if position_collision[1] > 0:
            action = Actions.TURNLEFT

        self.rotate(angle=rotation_angle, action=action)
        
        self.run()

    # DEFINITION OF STATES
    def init(self):
        """
        **This state initializes the thrusters**
        """

        print("Searching for launcher...")
        
        while self.target_object != OBJECT_INITIALIZATION:
            self.search_objects()

        # manter comentado para testes que somente 1 objeto é identificado
        # self.target_object = None

        print("Initializing...")

        self.thrusters = ThrustersControl()
        self.transition_to(State.SEARCH)
    
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
                self.thrusters.define_action({Actions.DOWN: 20})
                rotation_current = 0

            self.search_objects()

        # verificar qual objeto(os) encontrou e responder de acordo
        
        self.transition_to(State.CENTERING)

    def search_objects(self):
        """
        Checks if objects were found. Found saved in target_object
        """

        if self.yolo_ctrl.found_object():
            self.target_object = self.yolo_ctrl.greater_confidence_object()

    def rotate(self, angle = 0.785398, error_angle = 0.174533, action = Actions.TURNLEFT):
        gyro_current = self.sensors.get_gyro()
        gyro_old = None

        rotated = 0

        while fabs(rotated) < angle - error_angle:
            self.thrusters.define_action({action: 20})

            gyro_old = gyro_current
            gyro_current = self.sensors.get_gyro()
            delta_time = self.sensors.current_time - self.sensors.old_time

            rotated += delta_time * (gyro_current[2] + gyro_old[2]) / 2

        self.thrusters.define_action({action: 0})
    
    #testar
    def centering(self):
        """
        **This state defines the centralization procedure**
        """

        print("Centering...")

        lost_object = 0
        is_center = 0
        
        while not is_center:
            xyxy = self.yolo_ctrl.get_xyxy(self.target_object)

            if xyxy != None:
                actions = center_object(xyxy)

                # Mudar de dicionario para array (é mais rápido)

                self.thrusters.define_action({actions[1]: actions[2], actions[3]: actions[4]})

                is_center = actions[0]
            else:
                is_center = 1
                lost_object = 1
                print("Lost object!")
            
        if not lost_object:            
            self.next_state = State.ADVANCING
            self.transition_to(State.STABILIZING)
        else:
            self.transition_to(State.SEARCH)
    
    #testar
    def advancing(self):
        """
        **This state difines the advancement procedure**
        """

        print("Advancing...")

        lost_object = 0
        advance = 1

        while advance:
            xyxy = self.yolo_ctrl.get_xyxy(self.target_object)
            
            if xyxy != None:
                self.distance = calculate_distance(self.target_object, xyxy)
                action = advance_decision(self.distance)

                self.thrusters.define_action({action[1]: action[2]})

                advance = action[0]
            else:
                advance = 0
                lost_object = 1
                print("Lost object!")
        
        if not lost_object:
            self.next_state = State.STOP
            self.transition_to(State.STABILIZING)
        else:
            self.transition_to(State.SEARCH)

    def stabilizing(self):
        """
        **This state stabilizes the AUV**
        """

        print("Stabilizing...")

        is_stable = False

        while not is_stable:
            velocity = self.sensors.get_vel()

            actions = stabilizes(velocity)

            self.thrusters.define_action({actions[1]: actions[2]}) # x
            sleep(.5)
            self.thrusters.define_action({actions[3]: actions[4], actions[5]: actions[6]}) # y e z
            sleep(.5)

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

        self.yolo_ctrl.stop()

        self.thrusters.finish()
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
    Defines the constants for proportional controller and error between the interest variables
    """

    k_p = [.5, .5]
    k_i = [.5, .5]

    xm, ym = center(bounding_box)

    errors = [(xm - IMAGE_CENTER[0]), (ym - IMAGE_CENTER[1])]

    return set_power(k_p, k_i, errors, del_time)

def advance_set_power(distance = None, del_time = None):
    k_p = [4.5]
    k_i = [4.5]

    errors = [(distance - SAFE_DISTANCE)]

    return set_power(k_p, k_i, errors, del_time)

def stabilizes_set_power(velocity = None, del_time = None):
    k_p = [1.5, 1.5, 1.5]
    k_i = [1.5, 1.5, 1.5]

    return set_power(k_p, k_i, velocity, del_time)

def set_power(k_p = None, k_i = None, errors = None, del_time = None):
    """
    Defines the power that thrusters execution the moviment
    
    Multiplies the error by a constant to send the required power

    :param k_p: constants to the proportional controler
    :type k_p: Array of float
    :param errors: Errors values between detect value and target value
    :type errors: Array of float

    :return: A array of float with power values
    """

    values = [0 * len(errors)]

    for i in range(len(errors)):
        e = fabs(errors[i])
        values[i] = max(min(k_p[i] * e + k_i[i] * (e * del_time), 100), 0)
    
    return values

def center_object(xyxy):
    """
    Decides which movement to take based on the position of the object in the image.

    :param xyxy: x and y coordinates of the detected object
    :type xyxy: Array

    :return: Movement that the AUV must take
    """

    dir_h = None
    dir_v = None
    power_h = 0
    power_v = 0
    xm, ym = center(xyxy)

    if(xm >= 0 and xm <= IMAGE_WIDTH and ym >= 0 and ym <= IMAGE_HEIGHT):
        if(xm < IMAGE_CENTER[0] - (ERROR_CENTER / 2)):
            dir_h = Actions.LEFT
        elif(xm > IMAGE_CENTER[0] + (ERROR_CENTER / 2)):
            dir_h = Actions.RIGHT
        if(ym < IMAGE_CENTER[1] - (ERROR_CENTER / 2)):
            dir_v = Actions.UP
        elif(ym > IMAGE_CENTER[1] + (ERROR_CENTER / 2)):
            dir_v = Actions.DOWN

    power_h, power_v = set_power(bounding_box = xyxy)

    return [0 if dir_h is not None or dir_v is not None else 1, dir_h, power_h, dir_v, power_v]

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
    width_objects = {"obj1": 2, "Cube": .055} # Ex

    # Initializes the variable with invalid value to indicates error
    object_distance = -1

    if (object_class in width_objects) and (xyxy[2] - xyxy[0] != 0):
        # Image diagonal (in pixels)
        d = sqrt(pow(IMAGE_WIDTH, 2) + pow(IMAGE_HEIGHT, 2))

        # Diagonal field of view (in rad)
        a = (pi / 180) * 55

        # focal distance
        f = (d / 2) * (cos(a / 2) / sin(a / 2))

        object_distance = (f * width_objects[object_class]) / (xyxy[2] - xyxy[0])
        print(f"Object distance: {object_distance}")

    return object_distance

def advance_decision(object_distance):
    """
    Decides whether to advance to the object and the power that will be used

    :param object_distance: Distance between the AUV and the object
    :type object_distance: Int

    :return: Whether advance or no, action and power that must be used
    """
    
    action = None
    power = 0

    if(object_distance > SAFE_DISTANCE):
        action = Actions.FORWARD

        power = set_power(distance = object_distance)[0]
    
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

    action_x = defines_action(velocity[0], error_velocity[0], Actions.FORWARD, Actions.BACKWARD)
    action_y = defines_action(velocity[1], error_velocity[1], Actions.RIGHT, Actions.LEFT)
    action_z = defines_action(velocity[2], error_velocity[2], Actions.DOWN, Actions.UP)

    is_stable = 1 if action_x is None and action_y is None and action_z is None else 0

    if not is_stable:
        power_x, power_y, power_z = set_power(velocity = velocity)

    return [is_stable, action_x, power_x, action_y, power_y, action_z, power_z]

def defines_action(velocity, error_velocity, positive_action, negative_action):
    action = None

    if velocity > 0 + error_velocity:
        action = negative_action
    elif velocity < 0 - error_velocity:
        action = positive_action
    
    return action