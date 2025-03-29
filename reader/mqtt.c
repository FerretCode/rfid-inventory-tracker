#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"

#define DNS_OK 0
#define DNS_ERR 1

#define MQTT_OK 0
#define MQTT_ERR 0

typedef struct MQTT_CLIENT_T_ {
    ip_addr_t remote_addr;
    int remote_port;
    mqtt_client_t* mqtt_client;
    u32_t received;
    u32_t counter;
    u32_t reconnect;
} MQTT_CLIENT_T;

static MQTT_CLIENT_T* mqtt_client_init(int remote_port)
{
    MQTT_CLIENT_T* state = (MQTT_CLIENT_T*)calloc(1, sizeof(MQTT_CLIENT_T));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }

    state->received = 0;
    state->remote_port = remote_port;
    return state;
}

void dns_found(const char* name, const ip_addr_t* ip_addr, void* callback_arg)
{
    MQTT_CLIENT_T* state = (MQTT_CLIENT_T*)callback_arg;
    state->remote_addr = *ip_addr;
}

int run_dns_lookup(MQTT_CLIENT_T* state, char* hostname)
{
    cyw43_arch_lwip_begin();
    err_t err = dns_gethostbyname(hostname, &(state->remote_addr), dns_found, state);
    cyw43_arch_lwip_end();

    if (err == ERR_ARG) {
        printf("Failed to begin DNS query\n");
        return 1;
    }

    if (err == ERR_OK) {
        // no need to perform DNS lookup
        return 0;
    }

    while (state->remote_addr.addr == 0) {
        cyw43_arch_poll();
        sleep_ms(1);
    }

    return 0;
}

int mqtt_connect(MQTT_CLIENT_T* state)
{
    struct mqtt_connect_client_info_t ci;

    memset(&ci, 0, sizeof(ci));

    ci.client_id = "Reader"; // TODO: consider other static unique ID for multiple boards
    ci.client_user = NULL;
    ci.client_pass = NULL;
    ci.keep_alive = 0;
    ci.will_topic = NULL;
    ci.will_msg = NULL;
    ci.will_retain = 0;
    ci.will_qos = 0;

    // TODO: configure TLS

    const struct mqtt_connect_client_info_t* client_info = &ci;

    err_t err = mqtt_client_connect(state->mqtt_client, &(state->remote_addr), state->remote_port, NULL, NULL, client_info);
    if (err != ERR_OK) {
        printf("Error connecting to MQTT broker\n");
        return err;
    }

    return err;
}

int init_mqtt(char* ssid, char* pw, char* broker_hostname, int broker_port, uint8_t timeout)
{
    if (cyw43_arch_init()) {
        printf("Failed to initialize MQTT\n");
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to WiFi \n");
    if (cyw43_arch_wifi_connect_timeout_ms(
            ssid, pw, CYW43_AUTH_WPA2_AES_PSK, timeout)) {
        printf("Failed to connect to WiFi\n");
        return 1;
    } else {
        printf("Connected to WiFi");
    }

    MQTT_CLIENT_T* state = mqtt_client_init(broker_port);

    int res = run_dns_lookup(state, broker_hostname);
    if (res == DNS_ERR) {
        return res;
    }

    state->mqtt_client = mqtt_client_new();
    state->counter = 0;

    if (state->mqtt_client == NULL) {
        printf("Failed to create new MQTT client\n");
        return MQTT_ERR;
    }

    return 0;
}
