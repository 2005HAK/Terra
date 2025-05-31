#!/usr/bin/env python

import time
from pymavlink import mavutil

master = mavutil.mavlink_connection('127.0.0.1:14550')
count = 0
initial_time = time.time()

while count < 100:
	msg = master.recv_match()
	
	if msg:
		count += 1
		
final_time = time.time()

delta_time = final_time - initial_time

print(count / delta_time)
