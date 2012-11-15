using the cmake build:
This directory contains a cmake file which can be used to build gs_upi and the process3 toolchain. Here are the instructions to do an out-of-source build which compiles in ./build and installs the binaries into ./bin

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=../ -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make
make install

don't forget to do the make install command!


process3/rebuild

util/cdbtools/

LD_LIBRARY_PATH=/opt/qt2/lib wired

PATH=$PATH:~/devel/devel_3dyne_retro/gs_retro/bin _process3