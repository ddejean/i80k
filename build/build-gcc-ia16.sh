#!/bin/bash

set -e

BUILD_DIR=build-ia16
 
git clone --depth 1 https://github.com/tkchia/build-ia16.git ${BUILD_DIR}
pushd ${BUILD_DIR}
./fetch.sh gitlab.com shallow
./build.sh binutils prereqs gcc1
popd
