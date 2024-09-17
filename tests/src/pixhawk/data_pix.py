import pixhawk as px
from pymavlink import mavutil
import time

connection = mavutil.mavlink_connection('/dev/ttyACM0', baud = 115200)
i = 0
time1 = time.time()
while i < 1000:
    # msg = connection.recv_match(type='LOCAL_POSITION_NED', blocking=True)
    msg = connection.recv_match()

    if not msg:
        continue

    print(msg)
    i += 1
    
    """
    print(msg.vx)
    print(msg.vy)
    print(msg.vz)
    """

time2 = time.time()

var = time2 - time1

print(i / var)