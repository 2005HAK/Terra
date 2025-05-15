sudo rm -rf build/*
cd build
cmake ..
make -j$(nproc)
sudo ip addr add 192.168.0.1/24 dev eth0
sudo ip link set eth0 up
sudo ip link set eth0 up
sudo ip link set eth0 up
sudo ./run_auv
