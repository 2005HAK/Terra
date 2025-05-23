import control_motors as cm
import pygame

# Power limit (in %)
POWER = 10

# initializes the pygame module
pygame.init()

# Creation of the motors object
motors = cm.Motors()

def main():
    clock = pygame.time.Clock()
    joysticks = {}

    done = False
    while not done:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                done = True

            # recognizes wheter a joystick has been connected/disconnected
            if event.type == pygame.JOYDEVICEADDED:
                joy = pygame.joystick.Joystick(event.device_index)
                joysticks[joy.get_instance_id()] = joy
                print(f"Joystick {joy.get_instance_id()} connencted")

            if event.type == pygame.JOYDEVICEREMOVED:
                del joysticks[event.instance_id]
                print(f"Joystick {event.instance_id} disconnected")

        # Takes the values of the left (1) and right (3) analogues on the y-axis
        for joystick in joysticks.values():
            axis_ly = joystick.get_axis(1)
            axis_ry = joystick.get_axis(3)

            actions = {}

            # Decides movement and power based on how much the analog stick was moved
            if axis_ly < 0:
                actions["UP"] = -axis_ly * POWER
            else:
                actions["DOWN"] = axis_ly * POWER

            if axis_ry < 0:
                actions["FRONT"] = -axis_ry * POWER
            else:
                actions["BACK"] = axis_ry + POWER
            
            print(actions)

            # Passes the movements to the engines
            motors.define_action(actions)

        # Limit to 30 frames per second.
        clock.tick(30)

if __name__ == "__main__":
    main()

    # Close the pygame module
    pygame.quit()