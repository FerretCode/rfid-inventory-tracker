#include "item_registration.h"
#include "cJSON.h"
#include "mqtt.h"
#include "string.h"

void enqueue_task(Task** head, ITEM_T* item)
{
    struct Task* new_task = (struct Task*)malloc(sizeof(struct Task));
    if (new_task == NULL) {
        printf("Could not allocate new task to the queue\n");
        return;
    }

    new_task->item = item;
    new_task->next = NULL;

    if (*head == NULL) {
        *head = new_task;
    } else {
        struct Task* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }

        temp->next = new_task;
    }

    printf("Task enqueued for item %d.\n", item->item_id);
}

/*
 * Ensure that the returned item is freed after use.
 * Only the task will be freed in this function
 */
ITEM_T* dequeue_task(struct Task** head)
{
    if (*head == NULL) {
        return NULL;
    }

    struct Task* temp = *head;
    ITEM_T* item = temp->item;

    *head = temp->next;

    free(temp);
    return item;
}

void subscribe_incoming_items(MQTT_CLIENT_T* state, Task** head)
{
    mqtt_set_inpub_callback(
        state->mqtt_client,
        NULL,
        process_incoming_item_data,
        head);

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

        if (!arg) {
            printf("Failed to process item\n");
            return;
        }

        Task** head = (Task**)arg;

        ITEM_T* item = (ITEM_T*)malloc(sizeof(ITEM_T));

        cJSON* json = cJSON_Parse((const char*)data);
        if (json == NULL) {
            const char* err_ptr = cJSON_GetErrorPtr();
            if (err_ptr != NULL) {
                printf("Error parsing JSON payload: %s\n", err_ptr);
            }
            cJSON_Delete(json);
            return;
        }

        cJSON* item_id = cJSON_GetObjectItemCaseSensitive(json, "item_id");
        cJSON* tag_quantity = cJSON_GetObjectItemCaseSensitive(json, "tag_quantity");

        if (!cJSON_IsNumber(item_id) || !cJSON_IsNumber(tag_quantity)) {
            printf("Invalid JSON payload format\n");
            cJSON_Delete(item_id);
            cJSON_Delete(tag_quantity);
            cJSON_Delete(json);
            return;
        }

        item->item_id = cJSON_GetNumberValue(item_id);
        item->tag_quantity = cJSON_GetNumberValue(tag_quantity);

        enqueue_task(head, item);
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

    free(item);
    free(json_str);
}
