/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "photo_album.h"
#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// UI mode
typedef enum {
    UI_MODE_IMAGE,
    UI_MODE_VIDEO
} ui_mode_t;

// UI events
typedef enum {
    UI_EVENT_SWIPE_LEFT,
    UI_EVENT_SWIPE_RIGHT,
    UI_EVENT_SWIPE_UP,      // Volume up (MP4 mode only)
    UI_EVENT_SWIPE_DOWN,    // Volume down (MP4 mode only)
    UI_EVENT_LONG_PRESS,
    UI_EVENT_TAP,
    UI_EVENT_SETTINGS_CLOSE,
    UI_EVENT_SETTINGS_CANCEL
} ui_event_t;

// UI event callback
typedef void (*ui_event_cb_t)(ui_event_t event, void *user_data);

// UI manager functions
esp_err_t ui_manager_init(ui_event_cb_t event_cb, void *user_data);
esp_err_t ui_manager_deinit(void);
esp_err_t ui_manager_display_image(const decoded_image_t *image);
esp_err_t ui_manager_show_loading(void);
esp_err_t ui_manager_hide_loading(void);
esp_err_t ui_manager_show_settings(uint32_t current_interval);
esp_err_t ui_manager_hide_settings(void);
esp_err_t ui_manager_update_progress(int current, int total);
uint32_t ui_manager_get_selected_interval(void);

// Video mode support
esp_err_t ui_manager_switch_mode(ui_mode_t mode);
esp_err_t ui_manager_display_video_frame(const uint8_t *frame_buffer, uint32_t width, uint32_t height);

// Volume display (video mode)
esp_err_t ui_manager_show_volume(int volume_percent);

#ifdef __cplusplus
}
#endif 