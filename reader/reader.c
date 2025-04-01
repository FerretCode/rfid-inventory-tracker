#include "hardware/i2c.h"
#include "item_registration.h"
#include "mfrc522.h"
#include "mqtt.h"
#include "pico/cyw43_arch.h"
#include "pico/stdio.h"
#include "ssd1306.h"
#include "string.h"
#include <stdint.h>
#include <string.h>

#define CONFIRMATION_SWITCH_PIN 28
#define I2C_PORT i2c1
#define DISPLAY_SCL 15
#define DISPLAY_SDA 14

int main()
{

    int mqtt_timeout = 30000;

    stdio_init_all();
    i2c_init(I2C_PORT, 400000);

    // configure the confirmation switch
    gpio_init(CONFIRMATION_SWITCH_PIN);
    gpio_set_dir(CONFIRMATION_SWITCH_PIN, GPIO_IN);
    gpio_pull_up(CONFIRMATION_SWITCH_PIN);

    // configure the OLED display
    gpio_set_function(DISPLAY_SCL, GPIO_FUNC_I2C);
    gpio_set_function(DISPLAY_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(DISPLAY_SCL);
    gpio_pull_up(DISPLAY_SDA);

    ssd1306_t display;
    display.external_vcc = false;
    ssd1306_init(&display, 128, 32, 0x3C, I2C_PORT);
    ssd1306_clear(&display);

    sleep_ms(3000);

    ssd1306_draw_string(&display, 0, 0, 1, "Starting board...");
    ssd1306_show(&display);

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

    MFRC522Ptr_t mfrc = MFRC522_Init();
    PCD_Init(mfrc, spi0);

    int mode_comparison = strcmp(MODE, "reader");
    if (mode_comparison == 0) {
        printf("Starting in reader mode\n");
        uint8_t tag1[] = { 0x93, 0xE3, 0x9A, 0x92 };

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
        }
    } else {
        printf("Starting in writer mode \n");
        struct Task* queue = NULL;

        subscribe_incoming_items(state, &queue);

        ssd1306_clear(&display);
        ssd1306_draw_string(&display, 0, 0, 1, "Waiting for items.");
        ssd1306_show(&display);

        sleep_ms(2000);

        bool needs_item = true;
        bool can_publish = false;
        bool needs_tag = true;
        bool read_tag = false;
        ITEM_T* item = NULL;

        while (true) {
            cyw43_arch_poll();

            if (needs_item) {
                ITEM_T* to_process_item = dequeue_task(&queue);
                if (to_process_item != NULL) {
                    item = to_process_item;
                    printf("Item pulled from the queue: %d\n", item->item_id);
                    needs_item = false;
                }
            }

            if (item == NULL) {
                continue;
            }

            char quantity_string[32];
            snprintf(quantity_string, sizeof(quantity_string), "Tag Quantity: %d", item->tag_quantity);

            char item_name[32];
            // truncate item name if it exceeds the buffer size
            if (sizeof(item->item_name) <= 27) {
                snprintf(item_name, sizeof(item_name), "Item: %.27s", item->item_name);
            } else {
                snprintf(item_name, sizeof(item_name), "Item: %.24s...", item->item_name);
            }

            ssd1306_clear(&display);
            ssd1306_draw_string(&display, 0, 0, 1, item_name);
            ssd1306_draw_string(&display, 0, 15, 1, quantity_string);
            ssd1306_show(&display);

            if (needs_tag) {
                if (!PICC_IsNewCardPresent(mfrc)) {
                    continue;
                }

                needs_tag = false;
            }

            if (!read_tag) {
                printf("Selecting card\n\r");
                PICC_ReadCardSerial(mfrc);

                printf("PICC dump: \n\r");
                PICC_DumpToSerial(mfrc, &(mfrc->uid));

                printf("Uid is: ");
                for (int i = 0; i < 4; i++) {
                    printf("%x ", mfrc->uid.uidByte[i]);
                    item->tag[i] = mfrc->uid.uidByte[i];
                }
                printf("\n\r");
                read_tag = true;
            }

            bool confirmation_switch_state
                = gpio_get(CONFIRMATION_SWITCH_PIN);

            // if the tag has not been copied into the item yet, we don't want the button to trigger a publish
            if (!read_tag) {
                continue;
            }

            // the condition is inverted because the switch pin has a pullup resistor on it
            // thus the pin is pulled high when the switch is closed
            if (!confirmation_switch_state) {
                can_publish = true;
            }

            if (!can_publish) {
                continue;
            }

            printf("Publishing item after registration: %d\n", item->item_id);
            publish_item_registered(state, item);

            // reset state to wait for the next item
            ssd1306_clear(&display);
            ssd1306_draw_string(&display, 0, 0, 1, "Waiting for items.");
            ssd1306_show(&display);

            item = NULL;
            needs_item = true;
            needs_tag = true;
            can_publish = false;
            read_tag = false;

            sleep_ms(100);
        }
    }
}
