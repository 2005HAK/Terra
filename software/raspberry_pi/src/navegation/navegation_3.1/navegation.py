from enum import Enum, auto
import math as m
import pixhawk as px
import ia
import control_motors as cm
import threading
import time # importar somente time, é oque esta sendo usado
from AUVError import *

# Width and height of the image seen by the camera
IMAGE_WIDTH = 1280
IMAGE_HEIGHT = 720

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
        self.pixhawk = px.Pixhawk()
        self.ia = ia.Ia()
        self.target_object = None
        self.motors = None
        self.distance = None # passar o calculo e armazenamento de distancia para a pix

        # Update sensors data and detection data in parallel with the state machine
        self.sensor_thread = threading.Thread(target=self.update_sensors, daemon=True)
        self.detection_thread = threading.Thread(target=self.update_detection, daemon=True)
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
    def update_sensors(self):
        """
        Updates sensors data every **0.3 ms**
        """

        while True:
            self.pixhawk.update_data()
            time.sleep(0.3)

    # Não testado
    def update_detection(self):
        """
        Updates detection data every **0.3 ms**
        """

        while True:
            self.ia.update_data()
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
            self.errors_thread = threading.Thread(target=self.checks_errors, daemon=True)
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

    def direction_correction(self, acceleration):
        """
        Corrects the direction of the AUV by turning it 180º in relation to the crash location

        :param acceleration: Acceleration detected at the time of the crash
        :type acceleration: Float array
        """

        print("Correcting direction...")

        # 10º in rad
        error_angle = 0.174533

        position_collision = -acceleration

        # a = acos(x / sqrt(x^2 + y^2)) in degrees
        a = m.acos(position_collision[0] / m.sqrt(m.pow(position_collision[0], 2) + m.pow(position_collision[1], 2)))
        angle = a * m.pi / 180 # a in rad
        
        # Turn rigth by default
        action = "TURN RIGTH"

        # y > 0
        if position_collision[1] > 0:
            action = "TURN LEFT"

        gyro_current = self.pixhawk.get_gyro()
        gyro_old = None

        rotated = 0

        while m.abs(rotated) < angle - error_angle:
            self.motors.define_action({action: 20})

            gyro_old = gyro_current
            gyro_current = self.pixhawk.get_gyro()
            delta_time = self.pixhawk.current_time - self.pixhawk.old_time

            rotated += delta_time * (gyro_current[2] + gyro_old[2]) / 2
        
        self.run()

    # DEFINITION OF STATES
    def init(self):
        """
        **This state initializes the motors**
        """

        print("Initializing...")

        self.motors = cm.Motors()
        self.transition_to(State.SEARCH)
    
    def search(self):
        """
        **This state defines the search procedure**
        """

        print("Searching...")

        while not self.ia.found_object():
            self.motors.define_action({"FRONT": 20})

        self.target_object = self.ia.greater_confidence_object()

        # verificar qual objeto(os) encontrou e responder de acordo
        
        self.transition_to(State.CENTERING)

    def centering(self):
        """
        **This state defines the centralization procedure**
        """

        print("Centering...")

        self.transition_to(State.SEARCH)

        if self.ia.found_object():
            is_center = False
            
            while not is_center:
                # se o objeto deixar de ser identificado pela ia deve dar um break no while e em search tentar buscar o objeto novamente
                xyxy = self.ia.get_xyxy(self.target_object)

                actions = center_object(xyxy)

                # Mudar de dicionario para array (é mais rápido)

                self.motors.define_action({actions[1]: actions[2], actions[3]: actions[4]})
                time.sleep(.5)

                is_center = actions[0]
            
            self.next_state(State.ADVANCING)
            self.transition_to(State.STABILIZING)
    
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
                action = advance_decision(self.distance)

                self.motors.define_action({action[1]: action[2]})

                advance = action[0]
            
            self.next_state = State.STOP
            self.transition_to(State.STABILIZING)

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

# MUDAR ISSO!!! TA MUITO CENTRALIZADO
def set_power(bounding_box = None, distance = None, velocity = None):
    """
    Defines the power that motors execution the moviment

    Uses the diference into the object midpoint and image midpoint to calculate a value of power. The difference is multiplicate for weigth horizontal and vertical to return the value of power in each directions

    :param xyxy: x and y coordinates of the detected object
    :type xyxy: Array

    :return: A list with power_h, horizontal power, and power_v, vertical power, from 0-100%
    """

    # Defines the power maximum that motors can receive (in %)
    POWER_MAX = 25

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
        if(xm < IMAGE_CENTER[0] - (ERROR_CENTER / 2)):
            dir_h = "LEFT"
        elif(xm > IMAGE_CENTER[0] + (ERROR_CENTER / 2)):
            dir_h = "RIGHT"
        if(ym < IMAGE_CENTER[1] - (ERROR_CENTER / 2)):
            dir_v = "UP"
        elif(ym > IMAGE_CENTER[1] + (ERROR_CENTER / 2)):
            dir_v = "DOWN"

    power_h, power_v = set_power(bounding_box = xyxy)

    return [False if dir_h != "" or dir_v != "" else True, dir_h, power_h, dir_v, power_v]

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
    width_objects = {"obj1": 2, "obj2": 1.5} # Ex

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

def advance_decision(distance_object):
    """
    Decides whether to advance to the object and the power that will be used

    :param distance_object: Distance between the AUV and the object
    :type distance_object: Int

    :return: Whether advance or no, action and power that must be used
    """
    
    action = ""
    power = 0

    if(distance_object > SAFE_DISTANCE):
        action = "FORWARD"

        power = set_power(distance = distance_object)[0]
    
    return [True if action != "" else False, action, power]

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

def main():
    auv = AUVStateMachine()
    auv.run()

if __name__ == "__main__":
    main()