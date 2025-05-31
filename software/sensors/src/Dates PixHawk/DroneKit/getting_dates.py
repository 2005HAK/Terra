#!/usr/bin/env python
# -*- coding: utf-8 -*-

""" © Copyright 2015-2016, 3D Robotics. simple_goto.py: GUIDED mode "simple goto" example (Copter 
Only) Demonstrates how to arm and takeoff in Copter and how to navigate to points using 
Vehicle.simple_goto. Full documentation is provided at 
http://python.dronekit.io/examples/simple_goto.html """

from __future__ import print_function
import time
from dronekit import connect, VehicleMode, LocationGlobalRelative

# Set up option parsing to get connection string
import argparse
parser = argparse.ArgumentParser(description='Commands vehicle using vehicle.simple_goto.')
parser.add_argument('--connect', help="Vehicle connection target string. If not specified, SITL automatically started and used.")
args = parser.parse_args()

connection_string = args.connect
sitl = None

# Define Conection String
connection_string = '127.0.0.1:14550'

# Start SITL if no connection string specified
if not connection_string:
	import dronekit_sitl
	sitl = dronekit_sitl.start_default()
	connection_string = sitl.connection_string()

# Connect to the Vehicle
print('Connecting to vehicle on: %s' % connection_string)
vehicle = connect(connection_string, wait_ready=True)

print("Altitude: ", vehicle.location.global_relative_frame.alt)
time.sleep(5)
print("Ultimo heartbeat: ", vehicle.last_heartbeat)
print("Velocidade: ", vehicle.velocity)

# Close vehicle object before exiting script

print("Close vehicle object")
vehicle.close()

# Shut down simulator if it was started.
if sitl:
	sitl.stop()