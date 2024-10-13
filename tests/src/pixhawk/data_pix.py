import pixhawk as px
from pymavlink import mavutil
import time

connection = mavutil.mavlink_connection('COM7', baud = 115200)
i = 0
# time1 = time.time()

connection.mav.heartbeat_send(mavutil.mavlink.MAV_TYPE_GCS,
                                mavutil.mavlink.MAV_AUTOPILOT_INVALID, 0, 0, 0)

while i < 1000:
    msg = connection.recv_match(type='LOCAL_POSITION_NED', blocking=True)
    # msg = connection.recv_match()

    if not msg:
        continue

    print(msg)
    i += 1
    
    """
    print(msg.vx)
    print(msg.vy)
    print(msg.vz)
    """

# time2 = time.time()

# var = time2 - time1

# print(i / var)