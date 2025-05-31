#!/usr/bin/env python

"""
pega os dados de aceleração nos eixos x, y, z e os valores do giroscopio nos eixos x, y, z
xacc: [m/s²]
yacc: [m/s²]
zacc: [m/s²]
xgyro: [rad/s]
ygyro: [rad/s]
zgyro: [rad/s]
xmag: [mgauss]
ymag: [mgauss]
zmag: [mgauss]
"""

from pymavlink import mavutil
import time

master = mavutil.mavlink_connection('/dev/ttyACM0', baud=115200)

master.wait_heartbeat()

while True:
    msg = master.recv_match(type='HIGHRES_IMU', blocking=True)

    if not msg:
        continue

    print("xacc: %s" % msg.xacc)
    print("yacc: %s" % msg.yacc)
    print("zacc: %s" % msg.zacc)
    print("xgyro: %s" % msg.xgyro)
    print("ygyro: %s" % msg.ygyro)
    print("zgyro: %s" % msg.zgyro)
    print("ymag: %s" % msg.ymag)
    print("xmag: %s" % msg.xmag)
    print("zmag: %s" % msg.zmag)

    time.sleep(.1)