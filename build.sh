# make build dir
rm -rf ./build/
mkdir build && cd build
cmake -DCMAKE_CXX_FLAGS=-Iinclude ../
make
cd ..
# build/output