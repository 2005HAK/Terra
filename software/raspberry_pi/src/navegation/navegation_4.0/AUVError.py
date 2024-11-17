from time import asctime

class AUVError(Exception):
    """
    Generalization of errors or errors that are not identified
    """

    def __init__(self, time = asctime(), type = "Unidentified error", error_code = 95):
        self.time_error = time
        self.type_error = type
        self.error_code = error_code
        super().__init__(f"[{time}] (Error {error_code}): {type}")
        self.log_error()
    
    def log_error(self):
        """
        saves errors to a log file for later checking
        """

        with open("auv_errors.log", "a") as log_file:
            log_file.write(f"[{self.time_error}] (Error {self.error_code}): {self.type_error}\n")

class CollisionDetected(AUVError):
    """
    Detects AUV collisions
    """

    def __init__(self, acceleration = None, time = asctime()):
        self.acceleration = acceleration
        super().__init__(time, type = f"Collision detected based in sensor data: {acceleration} m/s²", error_code = 301)

class HighTempError(AUVError):
    """
    Generalization of temperature error or unidentified temperature problem
    """

    def __init__(self, time = asctime(), temperature = None, type = "High temperature detected", error_code = 95):
        self.temperature = temperature
        super().__init__(time, type, error_code)

class PixhawkHighTempError(HighTempError):
    """
    High temperature at pixhawk
    """

    def __init__(self, temperature = None, time = asctime()):
        super().__init__(time, temperature, type = f"Pixhawk temperature above threshold: {temperature} ºC", error_code = 332)

class RaspberryHighTempError(HighTempError):
    """
    High temperature in raspberry
    """
    
    def __init__(self, temperature = None, time = asctime()):
        super().__init__(time, temperature, type = f"Raspberry temperature above threshold: {temperature} ºC", error_code = 141)