import control_motors as cm
from pynput import keyboard

# Power limit (in %)
POWER = 10

# Creation of the motors object
motors = cm.Motors()

def on_press(key):
    """
    When pressing a key, a decision is made based on it and a command is sent to control the motors

    :param key: Key pressed
    """

    if key == keyboard.Key.space:
        print("UP")
        motors.define_action({"UP": POWER})
    if key == keyboard.Key.shift:
        print("DOWN")
        motors.define_action({"DOWN": POWER})
    if key == keyboard.Key.up:
        print("FRONT")
        motors.define_action({"FRONT": POWER})
    if key == keyboard.Key.down:
        print("BACK")
        motors.define_action({"BACK": POWER})
    if key == keyboard.Key.esc:
        return False
    
if __name__ == "__main__":
    # Listener creation to wait for a key to be pressed
    listener = keyboard.Listener(on_press = on_press)

    # Initialize the listener
    listener.start()

    # Ends the listener by pressing esc
    listener.join()