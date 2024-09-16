import pixhawk as px

sensors = px.Pixhawk()

while True:
    sensors.update_data()

    print(f"Acc: {sensors.get_acc()}")
    print(f"Gyro:{sensors.get_gyro()}")
    print(f"Mag: {sensors.get_mag()}")
    print(f"Vel: {sensors.get_vel()}")