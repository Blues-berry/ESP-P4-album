/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "esp_codec_dev.h"  // Add for audio device support
#include "driver/jpeg_decode.h"  // Add for JPEG decoder types

#ifdef __cplusplus
extern "C" {
#endif

#define APP_STREAM_JPEG_BUFFER_SIZE     (512 * 1024)  // 512KB JPEG buffer

/**
 * @brief Shared JPEG decoder manager for avoiding hardware conflicts
 */

/**
 * @brief Initialize shared JPEG decoder manager
 * @return ESP_OK on success, or an error code
 */
esp_err_t shared_jpeg_decoder_init(void);

/**
 * @brief Acquire shared JPEG decoder (thread-safe)
 * @param ret_handle Pointer to store the decoder handle
 * @return ESP_OK on success, or an error code
 */
esp_err_t shared_jpeg_decoder_acquire(jpeg_decoder_handle_t *ret_handle);

/**
 * @brief Release shared JPEG decoder (thread-safe)
 * @return ESP_OK on success, or an error code
 */
esp_err_t shared_jpeg_decoder_release(void);

/**
 * @brief Deinitialize shared JPEG decoder manager
 * @return ESP_OK on success, or an error code
 */
esp_err_t shared_jpeg_decoder_deinit(void);

/**
 * @brief Allocate JPEG-compatible memory for decoder output buffers
 * @param size Size of memory to allocate
 * @param allocated_size Actual allocated size
 * @return Pointer to allocated memory, or NULL on failure
 */
void* shared_jpeg_alloc_output_buffer(size_t size, size_t *allocated_size);

/**
 * @brief Free JPEG-compatible memory
 * @param ptr Pointer to memory to free
 */
void shared_jpeg_free_buffer(void *ptr);

/**
 * @brief Media stream adapter handle
 */
typedef struct app_stream_adapter_t* app_stream_adapter_handle_t;

/**
 * @brief JPEG decoder output format enumeration
 */
typedef enum {
    APP_STREAM_JPEG_OUTPUT_RGB565,    /*!< RGB565 format, 2 bytes per pixel */
    APP_STREAM_JPEG_OUTPUT_RGB888,    /*!< RGB888 format, 3 bytes per pixel */
} app_stream_jpeg_output_format_t;

/**
 * @brief JPEG decoder configuration structure
 */
typedef struct {
    app_stream_jpeg_output_format_t output_format;  /*!< JPEG decoder output format */
    bool bgr_order;                                 /*!< True for BGR order, false for RGB order */
} app_stream_jpeg_config_t;

/**
 * @brief Helper macro to create default JPEG configuration for RGB565 with BGR order
 */
#define APP_STREAM_JPEG_CONFIG_DEFAULT_RGB565() \
    { .output_format = APP_STREAM_JPEG_OUTPUT_RGB565, .bgr_order = true }

/**
 * @brief Helper macro to create default JPEG configuration for RGB888 with BGR order
 */
#define APP_STREAM_JPEG_CONFIG_DEFAULT_RGB888() \
    { .output_format = APP_STREAM_JPEG_OUTPUT_RGB888, .bgr_order = true }

/**
 * @brief Performance statistics structure
 */
typedef struct {
    float current_fps;            /*!< Current frames per second */
    uint32_t frames_processed;    /*!< Total frames processed */
} app_stream_stats_t;

/**
 * @brief Media frame callback function type
 *
 * @param buffer Pointer to the frame buffer
 * @param buffer_size Size of the frame buffer
 * @param width Frame width
 * @param height Frame height
 * @param frame_index Index of the current frame
 * @param user_data User data passed from configuration
 * @return ESP_OK on success, or an error code
 */
typedef esp_err_t (*app_stream_frame_cb_t)(uint8_t *buffer,
                                           uint32_t buffer_size,
                                           uint32_t width,
                                           uint32_t height,
                                           uint32_t frame_index,
                                           void *user_data);

/**
 * @brief Stream adapter initialization configuration structure
 */
typedef struct {
    app_stream_frame_cb_t frame_cb;                 /*!< Callback function for decoded frames */
    void *user_data;                                /*!< User data to be passed to frame callback */
    void **decode_buffers;                          /*!< Array of frame buffer pointers */
    uint32_t buffer_count;                          /*!< Number of frame buffers */
    uint32_t buffer_size;                           /*!< Size of each frame buffer */
    esp_codec_dev_handle_t audio_dev;               /*!< Audio device handle (NULL to disable audio) */
    app_stream_jpeg_config_t jpeg_config;           /*!< JPEG decoder configuration */
} app_stream_adapter_config_t;

/**
 * @brief Initialize stream adapter with unified configuration
 */
esp_err_t app_stream_adapter_init(const app_stream_adapter_config_t *config,
                                  app_stream_adapter_handle_t *ret_adapter);

/**
 * @brief Set media file with optional audio extraction
 */
esp_err_t app_stream_adapter_set_file(app_stream_adapter_handle_t handle,
                                      const char *filename,
                                      bool extract_audio);

/**
 * @brief Start playback
 */
esp_err_t app_stream_adapter_start(app_stream_adapter_handle_t handle);

/**
 * @brief Stop playback
 */
esp_err_t app_stream_adapter_stop(app_stream_adapter_handle_t handle);

/**
 * @brief Pause playback (keep position)
 */
esp_err_t app_stream_adapter_pause(app_stream_adapter_handle_t handle);

/**
 * @brief Resume playback from current position
 */
esp_err_t app_stream_adapter_resume(app_stream_adapter_handle_t handle);

/**
 * @brief Seek to position in milliseconds
 */
esp_err_t app_stream_adapter_seek(app_stream_adapter_handle_t handle, uint32_t position);

/**
 * @brief Get stream information
 */
esp_err_t app_stream_adapter_get_info(app_stream_adapter_handle_t handle,
                                      uint32_t *width, uint32_t *height,
                                      uint32_t *fps, uint32_t *duration);

/**
 * @brief Get performance statistics
 */
esp_err_t app_stream_adapter_get_stats(app_stream_adapter_handle_t handle,
                                       app_stream_stats_t *stats);

/**
 * @brief Cleanup and free resources
 */
esp_err_t app_stream_adapter_deinit(app_stream_adapter_handle_t handle);

#ifdef __cplusplus
}
#endif

/**
 * @brief Usage Examples:
 *
 * // New unified initialization interface (recommended)
 * app_stream_adapter_config_t config = {
 *     .frame_cb = frame_callback,
 *     .decode_buffers = buffers,
 *     .buffer_count = buffer_count,
 *     .buffer_size = buffer_size,
 *     .audio_dev = audio_device,  // Set to NULL to disable audio
 *     .jpeg_config = {
 *         .output_format = APP_STREAM_JPEG_OUTPUT_RGB888,
 *         .bgr_order = true  // Use BGR order for LCD compatibility
 *     }
 * };
 * app_stream_adapter_handle_t adapter;
 * app_stream_adapter_init(&config, &adapter);
 *
 * // Legacy separate initialization (still supported)
 * app_stream_adapter_handle_t adapter;
 * app_stream_adapter_init(frame_callback, buffers, buffer_count, buffer_size, &adapter);
 *
 * // Configure RGB888 format after initialization
 * app_stream_jpeg_config_t jpeg_config = {
 *     .output_format = APP_STREAM_JPEG_OUTPUT_RGB888,
 *     .bgr_order = false  // Use RGB order
 * };
 * app_stream_adapter_set_jpeg_config(adapter, &jpeg_config);
 *
 * // Initialize with audio support, then configure JPEG format
 * app_stream_adapter_init_with_audio(frame_callback, buffers, buffer_count, buffer_size, audio_dev, &adapter);
 * app_stream_adapter_set_jpeg_config(adapter, &jpeg_config);
 */
