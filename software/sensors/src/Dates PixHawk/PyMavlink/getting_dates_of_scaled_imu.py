#!/usr/bin/env python

"""
Pega os dados brutos da IMU
xacc: [mG]
yacc: [mG]
zacc: [mG]
xgyro: [mrad/s]
ygyro: [mrad/s]
zgyro: [mrad/s]
xmag: [mgauss]
ymag: [mgauss]
zmag: [mgauss]
"""

from pymavlink import mavutil

master = mavutil.mavlink_connection('127.0.0.1:14550')

while True:
    msg = master.recv_match()

    if not msg:
        continue
    if msg.get_type() == 'SCALED_IMU2':
        print("\n\n*****Mensagem de %s*****" % msg.get_type())
        print("Message: %s" % msg)