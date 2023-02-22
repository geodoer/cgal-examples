rd /s /q build
mkdir build
cd build
conan install .. -s compiler.version=15
cmake .. -G "Visual Studio 15 Win64"
pause