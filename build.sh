#!/bin/bash
STYLE="\e[36;1m\e[1m"
RESET="\e[0m"

TYPE=${TYPE:-Release}

mkdir -p release
cd release

echo -e "${STYLE}Compiling moonjit...${RESET}"
pushd ../moonjit/src || exit
make clean
make -j"${nproc}" XCFLAGS+="-DLUAJIT_ENABLE_LUA52COMPAT -DLUAJIT_ENABLE_GC64"
popd || exit

echo -e "${STYLE}Compiling RosaServer (${TYPE})...${RESET}"
cmake -DCMAKE_BUILD_TYPE=${TYPE} ..
make

mkdir -p output
cp -f ../moonjit/src/libluajit.so \
      RosaServer/librosaserver.so \
      RosaServerSatellite/rosaserversatellite \
      output/