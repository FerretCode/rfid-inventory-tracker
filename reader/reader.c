#include "item_registration.h"
#include "mfrc522.h"
#include "mqtt.h"
#include "pico/cyw43_arch.h"
#include "pico/stdio.h"
#include "string.h"
#include <stdint.h>
#include <string.h>

int main()
{

    int mqtt_timeout = 30000;

    stdio_init_all();

    sleep_ms(2000);

    printf("Starting reader...\n");
    printf(
        "WIFI_SSID: %s, WIFI_PASSWORD: %s, BROKER_HOSTNAME: %s, BROKER_PORT: %s\n",
        WIFI_SSID, WIFI_PASSWORD, BROKER_HOSTNAME, BROKER_PORT);

    MQTT_CLIENT_T* state
        = init_mqtt(
            WIFI_SSID,
            WIFI_PASSWORD,
            BROKER_HOSTNAME,
            (u16_t)atoi(BROKER_PORT),
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

    for (int i = 0; i < 50; i++) {
        cyw43_arch_poll();
        sleep_ms(100);
    }

    int mode_comparison = strcmp(MODE, "reader");
    if (mode_comparison == 0) {
        printf("Starting in reader mode\n");
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
        printf("Starting in writer mode \n");
        subscribe_incoming_items(state);

        while (true) {
            cyw43_arch_poll();
            sleep_ms(100);
        }
    }
}
