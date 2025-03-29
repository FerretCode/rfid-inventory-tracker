#include "mfrc522.h"
#include "mqtt.h"
#include "pico/stdio.h"
#include <stdint.h>
#include <string.h>

int main()
{

    char* broker_hostname = "test";
    int broker_port = 3000;
    uint8_t mqtt_timeout = 30000;

    stdio_init_all();

    MQTT_CLIENT_T* state = init_mqtt(
        WIFI_SSID,
        WIFI_PASSWORD,
        broker_hostname,
        broker_port,
        mqtt_timeout);

    // check if MQTT connection succeeded
    /* there is a chance that if the board could not connect to WiFi
     * that the initialization function could return a null pointer
     * hence the check
     */
    if (!state) {
        printf("Failed to initialize MQTT connection\n");
        return 1;
    }
    if (state->err != 0) {
        printf("Failed to initialize MQTT connection\n");
        return 1;
    }

    if (MODE == "reader") {
        uint8_t tag1[] = { 0x93, 0xE3, 0x9A, 0x92 };

        MFRC522Ptr_t mfrc = MFRC522_Init();
        PCD_Init(mfrc, spi0);

        sleep_ms(5000);

        while (1) {
            printf("Waiting for card\n\r");
            while (!PICC_IsNewCardPresent(mfrc))
                ;
            printf("Selecting card\n\r");
            PICC_ReadCardSerial(mfrc);

            printf("PICC dump: \n\r");
            PICC_DumpToSerial(mfrc, &(mfrc->uid));

            printf("Uid is: ");
            for (int i = 0; i < 4; i++) {
                printf("%x ", mfrc->uid.uidByte[i]);
            }
            printf("\n\r");

            if (memcmp(mfrc->uid.uidByte, tag1, 4) == 0) {
                printf("Authentication Success\n\r");
            } else {
                printf("Authentication Failed\n\r");
            }
        }
    } else {
    }
}
