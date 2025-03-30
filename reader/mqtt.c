#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/tcp.h"
#include "mqtt.h"
#include "pico/cyw43_arch.h"

#define DNS_OK 0
#define DNS_ERR 1

#define MQTT_OK 0
#define MQTT_ERR 1

MQTT_CLIENT_T* mqtt_client_init(u16_t remote_port)
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

const char* ip_addr_to_str(ip_addr_t ip)
{
    static char ip_str[16];

    snprintf(ip_str, sizeof(ip_str), "%u.%u.%u.%u",
        (ip.addr >> 0) & 0xFF,
        (ip.addr >> 8) & 0xFF,
        (ip.addr >> 16) & 0xFF,
        (ip.addr >> 24) & 0xFF);

    return ip_str;
}

void mqtt_connection_cb(mqtt_client_t* client, void* arg, mqtt_connection_status_t status)
{
    if (status != 0) {
        printf("Error during connection: err %d.\n", status);
    } else {
        printf("MQTT connected.\n");
    }
}

int mqtt_connect(MQTT_CLIENT_T* state)
{
    struct mqtt_connect_client_info_t ci;

    memset(&ci, 0, sizeof(ci));

    ci.client_id = "Reader"; // TODO: consider other static unique ID for multiple boards
    ci.client_user = NULL;
    ci.client_pass = NULL;
    ci.keep_alive = 60;
    ci.will_topic = NULL;
    ci.will_msg = NULL;
    ci.will_retain = 0;
    ci.will_qos = 0;

    // TODO: configure TLS

    const char* ip_addr_string = ip_addr_to_str(state->remote_addr);

    printf("Connecting to MQTT broker with ipaddr: %s, port: %d, user: %s, pass: %s\n",
        ip_addr_string,
        state->remote_port,
        ci.client_user,
        ci.client_pass);

    int wifi_status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    printf("WiFi Status before attempting MQTT broker connection: %d\n", wifi_status);

    err_t err = mqtt_client_connect(state->mqtt_client, &(state->remote_addr), state->remote_port, mqtt_connection_cb, state, &ci);
    if (err != ERR_OK) {
        printf("Error connecting to MQTT broker\n");
        return err;
    } else {
        printf("Attempting to connect to MQTT broker\n");
    }

    sleep_ms(5000);

    return err;
}

/**
 * Create a connection to the MQTT broker
 * Will return a MQTT client, but before use the err field should be checked
 * If there was an error initializing the connection, the client should not be used
 */
MQTT_CLIENT_T* init_mqtt(char* ssid, char* pw, char* broker_hostname, u16_t broker_port, int timeout)
{
    int wifi_init_result = cyw43_arch_init();
    if (wifi_init_result) {
        printf("Failed to initialize MQTT. Init result: %d\n", wifi_init_result);
        return NULL;
    }

    while (!cyw43_is_initialized(&cyw43_state)) {
        sleep_ms(1);
    }

    cyw43_arch_enable_sta_mode();

    sleep_ms(2000);

    printf("Connecting to WiFi with credentials ssid=%s, pw=%s\n", ssid, pw);
    int connection_result = cyw43_arch_wifi_connect_timeout_ms(
        ssid, pw, CYW43_AUTH_WPA2_AES_PSK, timeout);
    if (connection_result) {
        printf("Failed to connect to WiFi. Connection result: %d\n", connection_result);

        switch (connection_result) {
        case PICO_ERROR_BADAUTH:
            printf("Error authenticating to WiFi network\n");
            break;
        case PICO_ERROR_TIMEOUT:
            printf("Timeout authenticating to WiFi network\n");
            break;
        case PICO_ERROR_CONNECT_FAILED:
            printf("Other error connecting to WiFi network\n");
            break;
        }

        return NULL;
    } else {
        printf("Connected to WiFi\n");
    }

    MQTT_CLIENT_T* state = mqtt_client_init(broker_port);

    int res = run_dns_lookup(state, broker_hostname);
    if (res == DNS_ERR) {
        state->err = DNS_ERR;
        return state;
    }

    state->mqtt_client = mqtt_client_new();
    state->counter = 0;

    if (state->mqtt_client == NULL) {
        printf("Failed to create new MQTT client\n");
        state->err = MQTT_ERR;
        return state;
    }

    res = mqtt_connect(state);

    if (res != ERR_OK) {
        printf("Error connecting to MQTT broker\n");
        state->err = res;
        return state;
    }

    return state;
}
