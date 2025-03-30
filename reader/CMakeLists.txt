cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(reader_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()


add_executable(reader
    reader.c
    mqtt.c
    item_registration.c
    lib/pico-mfrc522/mfrc522.c
    lib/pico-ssd1306/ssd1306.c
    lib/cJSON/cJSON.c
)


target_compile_definitions(reader PRIVATE
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    BROKER_HOSTNAME=\"${BROKER_HOSTNAME}\"
    BROKER_PORT=\"${BROKER_PORT}\"
    MODE=\"writer\"
)

target_include_directories(reader PRIVATE
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/src/include/lwip
    ${PICO_SDK_PATH}/lib/lwip/src/include/netif
    ${PICO_SDK_PATH}/src/common
    ${PICO_SDK_PATH}/src/common/pico_stdlib/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_gpio/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_i2c/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_spi/include
    ${PICO_SDK_PATH}/src/common/pico_base/include
    lib/pico-mfrc522
    lib/pico-ssd1306
    lib/cJSON
    .
)

target_link_libraries(reader
    pico_stdlib
    hardware_adc
    pico_cyw43_arch_lwip_poll
    pico_lwip_mqtt
    pico_mbedtls
    pico_lwip_mbedtls
    hardware_i2c
    hardware_spi
)

pico_enable_stdio_usb(reader 1)
pico_enable_stdio_uart(reader 1)
pico_add_extra_outputs(reader)
