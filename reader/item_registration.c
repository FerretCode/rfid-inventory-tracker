#include "item_registration.h"
#include "cJSON.h"
#include "mqtt.h"
#include "string.h"

void subscribe_incoming_items(mqtt_client_t* client, void* arg, mqtt_connection_status_t status)
{
    if (status == MQTT_CONNECT_ACCEPTED) {
        mqtt_set_inpub_callback(
            client,
            process_incoming_item_pub,
            process_incoming_item_data,
            arg);
        mqtt_subscribe(
            client,
            SUB_TOPIC,
            1,
            NULL,
            NULL);
    } else {
        printf("Failed to subscribe to incoming item requetss\n");
        // TODO: consider reconnection logic
    }
}

void process_incoming_item_pub(void* arg, const char* topic, u32_t total_len)
{
    // ensure that the message is part of the correct topic
    int is_correct_topic = strcmp(topic, SUB_TOPIC);
    if (is_correct_topic) {
        inpub_id = 1;
    }

    inpub_id = -1;
}

void process_incoming_item_data(void* arg, const u8_t* data, u16_t total_len, u8_t flags)
{
    if (flags & MQTT_DATA_FLAG_LAST) {
        if (inpub_id != 1) {
            return;
        } else {
            // TODO: add to queue of items to show
        }
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
