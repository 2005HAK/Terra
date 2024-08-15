#!/usr/bin/env python

import time
from pymavlink import mavutil

master = mavutil.mavlink_connection('127.0.0.1:14550')

while True:
    try:
        print(master.recv_match().to_dict())
    except:
        pass
    time.sleep(0.1)z
