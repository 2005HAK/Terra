#!/bin/bash

echo "Atualizando pacotes..."
sudo apt update && sudo apt upgrade -y

echo "Instalando dependências..."
sudo apt install -y \
    cmake \
    make \
    g++ \
    pkg-config \
    libboost-all-dev \
    libjsoncpp-dev \
    libtinyxml2-dev \
    libopencv-dev \
    libcurl4-openssl-dev \
    libssl-dev

echo "Instalando WiringPi..."

sudo mkdir WiringPi
sudo git clone https://github.com/WiringPi/WiringPi.git WiringPi
cd WiringPi
sudo ./build
cd ..

echo "Baixando e instalando MAVSDK..."

wget https://github.com/mavlink/MAVSDK/releases/download/v3.0.0/libmavsdk-dev_3.0.0_debian12_arm64.deb
sudo dpkg -i libmavsdk-dev_3.0.0_debian12_arm64.deb
rm libmavsdk-dev_3.0.0_debian12_arm64.deb

wget json.hpp https://github.com/nlohmann/json/releases/latest/download/json.hpp

sudo mkdir /usr/include/nlohmann
sudo mv json.hpp /usr/include/nlohmann

echo "Instalação concluída!"

echo "Executando código"

rm -rf build/*
cd build
cmake ..
make
sudo ./run_auv
