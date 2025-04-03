#ifndef ITEM_REGISTRATION_H_
#define ITEM_REGISTRATION_H_

#include "mqtt.h"

#define SUB_TOPIC "item_registration"
#define PUB_TOPIC "item_registration_completed"

typedef struct ITEM_T_ {
    u32_t item_id;
    const char* item_name;
    u32_t tag_quantity;
    uint8_t tag[4];
} ITEM_T;

typedef struct Task {
    ITEM_T* item;
    struct Task* next;
} Task;

void enqueue_task(struct Task**, ITEM_T*);
ITEM_T* dequeue_task(struct Task**);

/**
 * Create the subscription to the incoming items topic
 */
void subscribe_incoming_items(MQTT_CLIENT_T* state, Task** head);

/**
 * The callback upon subscription
 */
void subscribed_topic(void* arg, err_t err);

/**
 * Publish the item with its tag back to the MQTT broker after the item is processed
 */
void publish_item_registered(MQTT_CLIENT_T* state, ITEM_T* item);

/**
 * The callback for when a new message is received from the broker
 * It will set the inpub_id based on whether it matches the incoming item topic
 */
void process_incoming_item_pub(void* arg, const char* topic, u32_t total_len);

/**
 * The callback for processing message data
 * If the message does not match the correct topic, it will be ignored
 */
void process_incoming_item_data(void* arg, const u8_t* data, u16_t total_len, u8_t flags);

void bytes_to_hex(const unsigned char* bytes, size_t len, char* out);

void hex_to_bytes(const char* hex, unsigned char* bytes, size_t* out_len);

#endif // ITEM_REGISTRATION_H_
