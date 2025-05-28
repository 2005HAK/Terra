from os import system
'''
Caso queira adicionar comandos que precisem de sudo use o f'echo{password} | sudo -S ...'
'''
password = input("Digite sua senha sudo: ")

comands = [
    'pip install ultralytics',
    f'echo {password} | sudo pip install Jetson.GPIO',
    f'echo {password} | sudo -S apt update',
    f'echo {password} | sudo -S apt install libboost-all-dev',
    f'echo {password} | sudo -S apt install nlohmann-json3-dev'
]

for comand in comands:
    system(comand)