#include "item_registration.h"
#include "cJSON.h"
#include "mqtt.h"
#include "string.h"

void subscribe_incoming_items(MQTT_CLIENT_T* state)
{
    mqtt_set_inpub_callback(
        state->mqtt_client,
        NULL,
        process_incoming_item_data,
        NULL);

    if (mqtt_client_is_connected(state->mqtt_client) == 1) {
        mqtt_subscribe(
            state->mqtt_client,
            SUB_TOPIC,
            1,
            subscribed_topic,
            NULL);
    } else {
        printf("MQTT client is not connected.\n");
    }
}

void subscribed_topic(void* arg, err_t err)
{
    printf("Subscription callback triggered\n");
    if (err != ERR_OK) {
        printf("Error subscribing. Err: %d\n", err);
    } else {
        printf("Successfully subscribed.\n");
    }
}

void process_incoming_item_data(void* arg, const u8_t* data, u16_t total_len, u8_t flags)
{
    if (flags & MQTT_DATA_FLAG_LAST) {
        printf("New item to register!\n");
        printf("Received Length: %d\n", total_len);
        printf("MQTT Payload: %.*s\n", total_len, (const char*)data);
        // TODO: add to queue of items to show
    }
}

void publish_item_registered(MQTT_CLIENT_T* state, ITEM_T* item)
{
    // encode the item as JSON
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "item_id", item->item_id);
    cJSON_AddNumberToObject(root, "tag_quantity", item->tag_quantity);

    cJSON* tag = cJSON_CreateArray();
    for (size_t i = 0; i < sizeof(item->tag); i++) {
        cJSON_AddItemToArray(tag, cJSON_CreateNumber(item->tag[i]));
    }
    cJSON_AddItemToObject(root, "tag", tag);

    char* json_str = cJSON_Print(root);
    cJSON_Delete(root);

    mqtt_publish(state->mqtt_client, PUB_TOPIC, json_str, strlen(json_str), 1, 0, NULL, NULL);

    free(json_str);
}
