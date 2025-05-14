sudo rm -rf build/*
cd build
cmake ..
make -j$(nproc)
sudo ./run_auv
