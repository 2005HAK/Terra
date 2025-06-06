sudo mv build/auv.log ~/Terra/logs/raspberry
sudo rm -rf build/*
cd build
cmake ..
make -j$(nproc)
sudo ./run_auv
