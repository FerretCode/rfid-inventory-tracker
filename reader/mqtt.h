#ifndef MQTT_H_
#define MQTT_H_

#include "err.h"
#include "lwip/apps/mqtt.h"

typedef struct MQTT_CLIENT_T_ {
    ip_addr_t remote_addr;
    u16_t remote_port;
    mqtt_client_t* mqtt_client;
    u32_t received;
    u32_t counter;
    u32_t reconnect;
    err_t err;
} MQTT_CLIENT_T;

MQTT_CLIENT_T* mqtt_client_init(u16_t remote_port);

void dns_found(const char* name, const ip_addr_t* ip_addr, void* callback_arg);

int run_dns_lookup(MQTT_CLIENT_T* state, char* hostname);

int mqtt_connect(MQTT_CLIENT_T* state);

MQTT_CLIENT_T* init_mqtt(char* ssid, char* pw, char* broker_hostname, u16_t broker_port, int timeout);

void mqtt_connection_cb(mqtt_client_t* client, void* arg, mqtt_connection_status_t status);

#endif // MQTT_H_
