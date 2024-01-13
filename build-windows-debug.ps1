mkdir -Force ./build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM="C:\\Program Files\\Ninja\\ninja.exe" -G "Ninja" ..
cmake --build .