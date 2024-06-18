#!/usr/bin/env python

"""
pega os dados de aceleração nos eixos x, y, z e os valores do giroscopio nos eixos x, y, z
xacc: [m/s²]
yacc: [m/s²]
zacc: [m/s²]
xgyro: [rad/s]
ygyro: [rad/s]
zgyro: [rad/s]
"""

from pymavlink import mavutil

master = mavutil.mavlink_connection('127.0.0.1:14550')

while True:
    msg = master.recv_match()

    if not msg:
        continue
    if msg.get_type() == 'SIMSTATE':
        print("\n\n*****Valores de %s*****" % msg.get_type())
        print("xacc: %s" % msg.xacc)
        print("yacc: %s²" % msg.yacc)
        print("zacc: %s" % msg.zacc)
        print("xgyro: %s" % msg.xgyro)
        print("ygyro: %s" % msg.ygyro)
        print("zgyro: %s" % msg.zgyro)