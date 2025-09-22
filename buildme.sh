if [ -d "build" ]; then
    rm -rf build
fi
mkdir -p build/sdsl-install
cd build
cmake ..
make