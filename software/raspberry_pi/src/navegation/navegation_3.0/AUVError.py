class AUVError(Exception):
    def __init__(self, time, type = "Unidentified error"):
        self.time_error = time
        self.type_error = type
        super().__init__(f"[{time}] {type}")

class CollisionDetected(AUVError):
    def __init__(self, acceleration, time):
        self.acceleration = acceleration
        super().__init__(time, type="Collision detected based in sensor data")