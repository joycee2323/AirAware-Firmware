#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/**
 * HTTPS detection uploader over the cellular PPP link.
 *
 * Reads detections from the in-memory FreeRTOS queue, batches them,
 * and POSTs to the backend.  When PPP is down, pushes overflow into
 * the SPIFFS-backed detection_queue.  Drains the SPIFFS queue when
 * PPP reconnects.
 *
 * Endpoint: POST /api/nodes/:device_id/detections
 * Auth:     X-Node-API-Key header
 */

/**
 * Start the uploader task.
 * @param detect_queue  FreeRTOS queue of odid_detection_t from distributor
 */
esp_err_t cellular_uploader_start(QueueHandle_t detect_queue);

/** Timestamp (ticks) of last successful upload (2xx), or 0 if never. */
TickType_t cellular_uploader_last_success(void);

/** Timestamp (ticks) of last HTTP response (any status), or 0 if never.
 *  Used by upload watchdog: reboot only when network is totally dead
 *  (no response at all), not when backend returns 4xx/5xx. */
TickType_t cellular_uploader_last_response(void);
