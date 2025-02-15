import pixhawk as px
from pymavlink import mavutil
import time

connection = mavutil.mavlink_connection('/dev/ttyACM0', baud = 115200)
i = 0
# time1 = time.time()

connection.mav.heartbeat_send(mavutil.mavlink.MAV_TYPE_SUBMARINE,
                                mavutil.mavlink.MAV_AUTOPILOT_INVALID, 0, 0, 0)

while 1:
    msg = connection.recv_match(blocking=True)
    # msg = connection.recv_match()

    if not msg:
        continue

    print(msg)
    # i += 1
    
    
    # print(msg.vx, msg.vy, msg.vz)


# time2 = time.time()

# var = time2 - time1

# print(i / var)
