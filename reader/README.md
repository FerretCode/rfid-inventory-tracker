# reader
The software and hardware for the reader board

# Components
- RPI Pico W main board
- MFRC522 Based RFID-RC522 RFID reader

# Driver Attributions
- https://github.com/BenjaminModica/pico-mfrc522 <- for the MFRC522 RFID module

# Building Instructions
```
mkdir -p build
cd build
cmake ..
make
```
- From there, the built `uf2` file can be flashed onto the RPI Pico W 

# Setup Instructions
- Change CMakeLists.txt accordingly
- Build the project and link `compile_commands` for use by the LSP
```
mkdir build
cd build
cmake ..
make
ln -s build/compile_commands.json .
```
