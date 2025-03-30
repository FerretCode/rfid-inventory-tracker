# reader
The software and hardware for the reader board

# Components
- RPI Pico W main board
- MFRC522 Based RFID-RC522 RFID reader

# Driver Attributions
- https://github.com/BenjaminModica/pico-mfrc522 <- for the MFRC522 RFID module

# Notes
- The MQTT implementation inside `lwip` requires the SNI patch
- The `lwip` MQTT implementation does not support persistent sessions. This means that the writer board must be online when new item registration requests are sent from the dashboard.


# Building Instructions
```
mkdir -p build
cd build
cmake -DPICO_BOARD=pico_w -DBROKER_HOSTNAME=mqtt_host -DBROKER_PORT=mqtt_port -DWIFI_SSID=wifi_ssid -DWIFI_PASSWORD=wifi_password ..
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
