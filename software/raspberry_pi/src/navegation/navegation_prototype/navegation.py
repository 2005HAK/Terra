import control_motors as cm
from pynput import keyboard

POWER = 10

motors = cm.Motors()

def on_press(key):
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


listener = keyboard.Listener(on_press=on_press)
listener.start()

listener.join()