/* SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/ip_addr.h"
#include "app_wifi.h"
#include "esp_mac.h"
#include "sdkconfig.h"

/* Default WiFi configuration */
#define DEFAULT_AP_SSID       CONFIG_WIFI_AP_SSID
#define DEFAULT_AP_PASSWORD   CONFIG_WIFI_AP_PASSWORD
#define DEFAULT_AP_CHANNEL    1
#define DEFAULT_MAX_STA_CONN  CONFIG_WIFI_AP_MAX_CLIENTS
#define DEFAULT_IP_ADDR       "192.168.4.1"

#define WIFI_SSID_LEN         32
#define WIFI_PASSWORD_LEN     64

static const char *TAG = "wifi";

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

static void wifi_init_softap(esp_netif_t *wifi_netif)
{
    // Set custom IP if needed
    if (strcmp(DEFAULT_IP_ADDR, "192.168.4.1")) {
        esp_netif_ip_info_t ip;
        memset(&ip, 0, sizeof(esp_netif_ip_info_t));
        ip.ip.addr = ipaddr_addr(DEFAULT_IP_ADDR);
        ip.gw.addr = ipaddr_addr(DEFAULT_IP_ADDR);
        ip.netmask.addr = ipaddr_addr("255.255.255.0");
        ESP_ERROR_CHECK(esp_netif_dhcps_stop(wifi_netif));
        ESP_ERROR_CHECK(esp_netif_set_ip_info(wifi_netif, &ip));
        ESP_ERROR_CHECK(esp_netif_dhcps_start(wifi_netif));
    }

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = DEFAULT_AP_SSID,
            .ssid_len = strlen(DEFAULT_AP_SSID),
            .password = DEFAULT_AP_PASSWORD,
            .max_connection = DEFAULT_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .channel = DEFAULT_AP_CHANNEL,
        },
    };

    if (strlen(DEFAULT_AP_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

    ESP_LOGI(TAG, "WiFi AP initialized. SSID:%s password:%s channel:%d",
             DEFAULT_AP_SSID, DEFAULT_AP_PASSWORD, DEFAULT_AP_CHANNEL);
}

esp_err_t app_wifi_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize networking stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create AP netif
    esp_netif_t *wifi_ap_netif = esp_netif_create_default_wifi_ap();

    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handler
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    // Set WiFi mode to AP
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    // Configure AP
    wifi_init_softap(wifi_ap_netif);

    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started successfully");
    return ESP_OK;
}

esp_err_t app_wifi_get_ip_info(esp_netif_ip_info_t *ip_info)
{
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (netif == NULL) {
        return ESP_FAIL;
    }

    return esp_netif_get_ip_info(netif, ip_info);
}
