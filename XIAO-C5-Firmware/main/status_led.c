#include "status_led.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "STATUS_LED";

#define LED_RED_GPIO    12      /* header D7 */
#define LED_GREEN_GPIO  8       /* header D8 */

static esp_timer_handle_t s_blink_timer;
static bool s_blink_on;

static void set_rgb(bool red, bool green)
{
    gpio_set_level(LED_RED_GPIO, red ? 1 : 0);
    gpio_set_level(LED_GREEN_GPIO, green ? 1 : 0);
}

static void blink_cb(void *arg)
{
    s_blink_on = !s_blink_on;
    set_rgb(s_blink_on, s_blink_on);   /* yellow when on */
}

esp_err_t status_led_init(void)
{
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << LED_RED_GPIO) | (1ULL << LED_GREEN_GPIO),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&io);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "gpio_config failed: %s", esp_err_to_name(err));
        return err;
    }

    const esp_timer_create_args_t targs = {
        .callback = blink_cb,
        .name     = "led_blink",
    };
    err = esp_timer_create(&targs, &s_blink_timer);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "blink timer create failed: %s", esp_err_to_name(err));
    }

    set_rgb(false, false);
    ESP_LOGI(TAG, "init: red=GPIO%d green=GPIO%d", LED_RED_GPIO, LED_GREEN_GPIO);
    return ESP_OK;
}

void status_led_set(status_led_state_t state)
{
    /* Stop any active blink timer before changing state */
    if (s_blink_timer) {
        esp_timer_stop(s_blink_timer);
    }

    switch (state) {
    case STATUS_LED_OFF:
        set_rgb(false, false);
        break;
    case STATUS_LED_GREEN:
        set_rgb(false, true);
        break;
    case STATUS_LED_YELLOW:
        set_rgb(true, true);
        break;
    case STATUS_LED_RED:
        set_rgb(true, false);
        break;
    case STATUS_LED_BLINK_YELLOW:
        s_blink_on = true;
        set_rgb(true, true);
        if (s_blink_timer) {
            esp_timer_start_periodic(s_blink_timer, 500000); /* 500 ms = 1 Hz */
        }
        break;
    }
    ESP_LOGI(TAG, "state → %d", (int)state);
}
