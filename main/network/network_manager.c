/* SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "network_manager.h"
#include "app_wifi.h"
#include "app_http_server.h"
#include "photo_album.h"
#include "esp_log.h"

static const char *TAG = "network_mgr";

// HTTP file uploaded/delete callback
static void http_file_uploaded_cb(const char *filepath)
{
    if (filepath) {
        ESP_LOGI(TAG, "File uploaded via HTTP: %s", filepath);
    } else {
        ESP_LOGI(TAG, "File deleted via HTTP");
    }
    // Refresh photo album asynchronously by posting event to UI task
    photo_album_refresh();
}

// Static flag to prevent multiple initializations
static bool s_network_manager_initialized = false;

esp_err_t network_manager_init(void)
{
    if (s_network_manager_initialized) {
        ESP_LOGW(TAG, "Network manager already initialized");
        return ESP_OK;
    }

    esp_err_t ret;

    // Initialize WiFi
    ESP_LOGI(TAG, "Initializing WiFi...");
    ret = app_wifi_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "WiFi init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "WiFi initialized successfully");

    // Start HTTP file server
    ESP_LOGI(TAG, "Starting HTTP file server...");
    ret = start_file_server("/sdcard/photos", http_file_uploaded_cb);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "HTTP server start failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "HTTP server started successfully");

    // Display connection information
    esp_netif_ip_info_t ip_info;
    if (app_wifi_get_ip_info(&ip_info) == ESP_OK) {
        ESP_LOGI(TAG, "WiFi AP IP: " IPSTR, IP2STR(&ip_info.ip));
        ESP_LOGI(TAG, "Upload URL: http://" IPSTR, IP2STR(&ip_info.ip));
        ESP_LOGI(TAG, "Modern UI: http://" IPSTR "/modern_upload.html", IP2STR(&ip_info.ip));
    }

    s_network_manager_initialized = true;
    return ESP_OK;
}
