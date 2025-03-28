#include "mfrc522.h"
#include "pico/stdio.h"

int main()
{
    stdio_init_all();

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
}
