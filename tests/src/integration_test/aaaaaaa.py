import pigpio
import time

# Connect to the pigpio daemon
pi = pigpio.pi()

if not pi.connected:
    print("Unable to connect to pigpio daemon.")
else:
    print("Connected to pigpio daemon.")
    
    # Test GPIO pin 17 (change this to your desired pin)
    pin = 17  
    pi.set_mode(pin, pigpio.OUTPUT)
    
    # Blink the LED
    try:
        for _ in range(10):
            pi.write(pin, 1)  # Set pin high
            time.sleep(1)
            pi.write(pin, 0)  # Set pin low
            time.sleep(1)
    finally:
        pi.stop()
