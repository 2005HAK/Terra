import pixhawk as px

sensors = px.Pixhawk()

while True:
    sensors.update_data()

    print(sensors.get_acc())