import pygame
import time

pygame.init()

def main():
    clock = pygame.time.Clock()
    joysticks = {}

    done = False
    while not done:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                done = True

            if event.type == pygame.JOYDEVICEADDED:
                joy = pygame.joystick.Joystick(event.device_index)
                joysticks[joy.get_instance_id()] = joy
                print(f"Joystick {joy.get_instance_id()} connencted")

            if event.type == pygame.JOYDEVICEREMOVED:
                del joysticks[event.instance_id]
                print(f"Joystick {event.instance_id} disconnected")

        for joystick in joysticks.values():
            axes = joystick.get_numaxes()

            axis_ly = joystick.get_axis(1)
            axis_ry = joystick.get_axis(3)
            print(f"axis y left: {axis_ly}")
            print(f"axis y right: {axis_ry}")
            time.sleep(.5)

        # Limit to 30 frames per second.
        clock.tick(30)

if __name__ == "__main__":
    main()

    pygame.quit()