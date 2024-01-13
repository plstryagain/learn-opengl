#!/bin/bash

mkdir -p ./build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Debug -DGLM_ROOT="$GLM_ROOT" ..
cmake --build .