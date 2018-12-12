#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#include "app_wifi.h"
#include "salty_transport.h"

#define LOG_TAG "app_main"

void app_main()
{
    esp_err_t nvs_ret = nvs_flash_init();
    if (nvs_ret == ESP_ERR_NVS_NO_FREE_PAGES || nvs_ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      nvs_ret = nvs_flash_init();
    }

    app_wifi_initialise();
    app_wifi_wait_connected();

    salty_transport_config_t salty_config = {
        .uri = "ws://echo.websocket.org"
    };

    salty_trans_init(&salty_config);
    int ret = -1;
    if((ret = salty_trans_connect()) < 0) {
        ESP_LOGE(LOG_TAG, "Failed to connect server, returned %d!", ret);
        return;
    } else {
        ESP_LOGI(LOG_TAG, "WS server connected!");
    }

    char text[] = "Hello websocket!";
    char *send_buf = strdup(text);
    size_t length = strlen(text) + 1;
    char recv_buf[30] = {'\0'};

    while(true) {
        ESP_LOGI(LOG_TAG, "Sending \"%s\"", send_buf);
        if((ret = salty_trans_write(send_buf, length)) < 0) {
            ESP_LOGE(LOG_TAG, "Failed to send message, returned %d!", ret);
            return;
        } else {
            ESP_LOGI(LOG_TAG, "Message sent");
        }

        send_buf = memcpy(send_buf, &text, length);
        vTaskDelay(500/portTICK_PERIOD_MS); // Wait for a while before receiving
        
        if((ret = salty_trans_read(recv_buf, length)) < 0) {
            ESP_LOGE(LOG_TAG, "Failed to receive message, returned %d!", ret);
            return;
        } else {
            ESP_LOGI(LOG_TAG, "Message received: %s, len: %d", recv_buf, ret);
        }

        memset(&recv_buf, '\0', 30);

        vTaskDelay(1000/portTICK_PERIOD_MS); // Loop every 1s
    }
}
