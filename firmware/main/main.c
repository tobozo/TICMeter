/**
 * @file main.cpp
 * @author Dorian Benech
 * @brief
 * @version 1.0
 * @date 2023-10-11
 *
 * @copyright Copyright (c) 2023
 *
 */

/*==============================================================================
 Local Include
===============================================================================*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#include "freertos/timers.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "cJSON.h"
#include "string.h"
#include "sdkconfig.h"
#include "esp_ota_ops.h"
#include "esp_pm.h"
#include "esp_private/esp_clk.h"

#include "common.h"
#include "linky.h"
#include "main.h"
#include "config.h"
#include "wifi.h"
#include "shell.h"
#include "mqtt.h"
#include "gpio.h"
#include "web.h"
#include "zigbee.h"
#include "tuya.h"
#include "ota.h"

#include "esp_heap_trace.h"
#include "esp_err.h"

/*==============================================================================
 Local Define
===============================================================================*/
#define MAIN_TAG "MAIN"

/*==============================================================================
 Local Macro
===============================================================================*/

/*==============================================================================
 Local Type
===============================================================================*/

/*==============================================================================
 Local Function Declaration
===============================================================================*/
void temp_loop(void *pvParameters);
/*==============================================================================
Public Variable
===============================================================================*/
TaskHandle_t fetchLinkyDataTaskHandle = NULL;
TaskHandle_t noConfigLedTaskHandle = NULL;
uint32_t main_sleep_time = 99999;
/*==============================================================================
 Local Variable
===============================================================================*/

/*==============================================================================
Function Implementation
===============================================================================*/
void app_main(void)
{
  ESP_LOGI(MAIN_TAG, "Starting TICMeter...");
  shell_wake_reason();
  gpio_init_pins();
  config_begin();
  gpio_boot_led_pattern();
  linky_init(MODE_HIST, RX_LINKY);
  xTaskCreate(gpio_pairing_button_task, "gpio_pairing_button_task", 8192, NULL, PRIORITY_PAIRING, NULL); // start push button task
  shell_init();                                                                                          // init shell
  wifi_init();                                                                                           // init wifi

  // linky_want_debug_frame = 2; // TODO: remove this

  // xTaskCreate(temp_loop, "test_task", 8 * 1024, NULL, 1, NULL); // start linky task

  // if (gpio_get_vusb() > 3 && config_values.mode == MODE_TUYA && config_values.tuyaBinded == 2) //
  // {
  //   ESP_LOGI(MAIN_TAG, "Tuya pairing");
  //   xTaskCreate(tuya_pairing_task, "tuya_pairing_task", 8192, NULL, 1, NULL); // start tuya pairing task
  //   while (config_values.tuyaBinded == 2)
  //   {
  //     vTaskDelay(1000 / portTICK_PERIOD_MS);
  //   }
  // }

  if (config_verify())
  {
    xTaskCreate(gpio_led_task_no_config, "gpio_led_task_no_config", 4 * 1024, NULL, PRIORITY_LED_NO_CONFIG, &noConfigLedTaskHandle); // start no config led task
    ESP_LOGW(MAIN_TAG, "No config found. Waiting for config...");
    while (config_verify())
    {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    if (config_values.mode == MODE_WEB)
    {
      ESP_LOGI(MAIN_TAG, "Config found, restarting... in 5s");
      vTaskDelay(5000 / portTICK_PERIOD_MS); // wait 5s to be sure that the web page is sent
      esp_restart();
    }
  }

  // check if VCondo is too low and go to deep sleep
  // the BOOT_PIN is used to prevent deep sleep when the device is plugged to a computer for debug
  if (gpio_get_vusb() < 3 && gpio_get_vcondo() < 3.5 && config_values.sleep && gpio_get_level(BOOT_PIN))
  {
    ESP_LOGI(MAIN_TAG, "VCondo is too low, going to deep sleep");
    esp_sleep_enable_timer_wakeup(20 * 1000000); // 10 second
    esp_deep_sleep_start();
  }

  switch (config_values.mode)
  {
  case MODE_WEB:
    // connect to wifi
    if (wifi_connect())
    {
      wifi_get_timestamp();               // get timestamp from ntp server
      wifi_http_get_config_from_server(); // get config from server
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      if (gpio_get_vusb() > 3)
      {
        ota_version_t version;
        ota_get_latest(&version);
      }
      wifi_disconnect();
    }
    break;
  case MODE_MQTT:
  case MODE_MQTT_HA:
    // connect to wifi
    if (wifi_connect())
    {
      mqtt_init();          // init mqtt
      wifi_get_timestamp(); // get timestamp from ntp server
      if (gpio_get_vusb() > 3)
      {
        ota_version_t version;
        ota_get_latest(&version);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      wifi_disconnect();
    }
    break;
  case MODE_ZIGBEE:
    if (gpio_get_vusb() < 3)
    {
      ESP_LOGW(MAIN_TAG, "Enable PM");
      esp_err_t rc = ESP_OK;
#ifdef CONFIG_PM_ENABLE
      int cur_cpu_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;
      esp_pm_config_t pm_config = {
        .max_freq_mhz = cur_cpu_freq_mhz,
        .min_freq_mhz = 10,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
        .light_sleep_enable = true
#endif
      };
      rc = esp_pm_configure(&pm_config);
#endif
      if (rc != ESP_OK)
      {
        ESP_LOGE(MAIN_TAG, "esp_pm_configure failed: 0x%x", rc);
      }
    }

    if (!linky_update())
    {
      ESP_LOGE(MAIN_TAG, "Cant find Linky: dont init zigbee");
    }
    else
    {
      zigbee_init_stack();
    }

    break;
  case MODE_TUYA:
    if (config_values.pairing_state != TUYA_PAIRED)
    {
      ESP_LOGW(MAIN_TAG, "Tuya not paired.");
      break;
    }
    if (wifi_connect())
    {
      tuya_init();
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      wifi_disconnect();
      vTaskSuspend(tuyaTaskHandle);
    }
    break;
  default:
    break;
  }
  // start linky fetch task
  xTaskCreate(main_fetch_linky_data_task, "main_fetch_linky_data_task", 16 * 1024, NULL, PRIORITY_FETCH_LINKY, &fetchLinkyDataTaskHandle); // start linky task
}
void main_fetch_linky_data_task(void *pvParameters)
{
#define MAX_DATA_INDEX 5
  LinkyData dataArray[MAX_DATA_INDEX];
  unsigned int dataIndex = 0;

  uint32_t last_heap = esp_get_free_heap_size();
  uint64_t next_update_check = wifi_get_timestamp();

  int32_t fetching_time = 0;
  switch (config_values.mode)
  {
  case MODE_WEB:
    fetching_time = 10;
    break;
  case MODE_MQTT:
  case MODE_MQTT_HA:
    fetching_time = 10;
    break;
  case MODE_ZIGBEE:
    fetching_time = 2;
    break;
  case MODE_TUYA:
    fetching_time = 10;
    break;
  default:
    fetching_time = 10;
    break;
  }

  while (1)
  {
    main_sleep_time = abs(config_values.refreshRate - fetching_time);
    ESP_LOGI(MAIN_TAG, "Waiting for %ld seconds", main_sleep_time);
    while (main_sleep_time > 0)
    {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      main_sleep_time--;
      if (gpio_get_vusb() < 3 && config_values.sleep && config_values.mode != MODE_ZIGBEE)
      {
        ESP_LOGI(MAIN_TAG, "USB disconnected, going to sleep for %ld seconds", main_sleep_time);
        uart_set_wakeup_threshold(UART_NUM_0, 3);
        esp_sleep_enable_uart_wakeup(UART_NUM_0);
        esp_sleep_enable_ext1_wakeup(1ULL << V_USB_PIN, ESP_EXT1_WAKEUP_ANY_HIGH);
        esp_sleep_enable_timer_wakeup(main_sleep_time * 1000000); // wait for refreshRate seconds before next loop
        main_sleep_time = 0;
        esp_light_sleep_start();
      }
    }
    ESP_LOGI(MAIN_TAG, "-----------------------------------------------------------------");
    ESP_LOGI(MAIN_TAG, "Waking up, VCondo: %f", gpio_get_vcondo());
    if (!linky_update() ||
        !linky_presence())
    {
      ESP_LOGE(MAIN_TAG, "Linky update failed");
      gpio_start_led_pattern(PATTERN_LINKY_ERR);
    }

    switch (config_values.mode)
    {
    case MODE_WEB:
    {
      // send data to web server
      if (dataIndex >= MAX_DATA_INDEX)
      {
        dataIndex = 0;
      }
      dataArray[dataIndex] = linky_data;
      dataArray[dataIndex++].timestamp = wifi_get_timestamp();
      ESP_LOGI(MAIN_TAG, "Data stored: %d - BASE: %lld", dataIndex, dataArray[0].timestamp);
      if (dataIndex > 2)
      {
        char json[1024] = {0};
        web_preapare_json_data(dataArray, dataIndex, json, sizeof(json));
        ESP_LOGI(MAIN_TAG, "Sending data to server");
        if (wifi_connect())
        {
          ESP_LOGI(MAIN_TAG, "POST: %s", json);
          wifi_send_to_server(json);
        }
        if (gpio_get_vusb() > 3)
        {
          ota_version_t version;
          ota_get_latest(&version);
        }
        wifi_disconnect();
        dataIndex = 0;
      }
      break;
    }
    case MODE_MQTT:
    case MODE_MQTT_HA: // send data to mqtt server
    {
      linky_free_heap_size = esp_get_free_heap_size();
      linky_uptime = esp_timer_get_time() / 1000000;
      uint8_t ret = mqtt_prepare_publish(&linky_data);
      if (ret == 0)
      {
        ESP_LOGE(MAIN_TAG, "Some data will not be sent, but we continue");
      }
      ret = wifi_connect();
      if (ret == 0)
      {
        ESP_LOGE(MAIN_TAG, "Wifi connection failed");
        goto send_error;
      }
      ESP_LOGI(MAIN_TAG, "Sending data to MQTT");
      ret = mqtt_send();
      if (ret == 0)
      {
        ESP_LOGE(MAIN_TAG, "MQTT send failed");
        goto send_error;
      }

      if (gpio_get_vusb() > 3 && next_update_check < wifi_get_timestamp())
      {
        ESP_LOGI(MAIN_TAG, "Checking for update");
        next_update_check = wifi_get_timestamp() + 3600;
        ota_version_t version;
        ota_get_latest(&version);
      }
      wifi_disconnect();
      gpio_start_led_pattern(PATTERN_SEND_OK);
      break;

    send_error:
      wifi_disconnect();
      gpio_start_led_pattern(PATTERN_SEND_ERR);
      break;
    }
    case MODE_TUYA:
      if (wifi_connect())
      {
        ESP_LOGI(MAIN_TAG, "Sending data to TUYA");
        resumeTask(tuyaTaskHandle); // resume tuya task
        if (tuya_wait_event(TUYA_EVENT_MQTT_CONNECTED, 10000))
        {
          ESP_LOGE(MAIN_TAG, "Tuya MQTT ERROR");
          gpio_start_led_pattern(PATTERN_SEND_ERR);
          goto tuya_disconect;
        }

        if (tuya_send_data(&linky_data))
        {
          ESP_LOGE(MAIN_TAG, "Tuya SEND ERROR");
          gpio_start_led_pattern(PATTERN_SEND_ERR);
          goto tuya_disconect;
        }
      tuya_disconect:
        // tuya_stop();
        // tuya_wait_event(TUYA_EVENT_MQTT_DISCONNECT, 5000);
        if (gpio_get_vusb() > 3 && next_update_check < wifi_get_timestamp())
        {
          ESP_LOGI(MAIN_TAG, "Checking for update");
          next_update_check = wifi_get_timestamp() + 3600;
          ota_version_t version;
          ota_get_latest(&version);
        }
        wifi_disconnect();
        suspendTask(tuyaTaskHandle);
        gpio_start_led_pattern(PATTERN_SEND_OK);
      }
      break;
    case MODE_ZIGBEE:
      zigbee_send(&linky_data);
      break;
    default:
      break;
    }
    ESP_LOGW(MAIN_TAG, "Free heap memory: %ld", esp_get_free_heap_size());
    ESP_LOGW(MAIN_TAG, "Heap diff: %ld", last_heap - esp_get_free_heap_size());
    last_heap = esp_get_free_heap_size();
  }
}

void temp_loop(void *pvParameters)
{

  esp_err_t rc = ESP_OK;
  int cur_cpu_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;
  esp_pm_config_t pm_config = {
      .max_freq_mhz = 10,
      .min_freq_mhz = 10,
  };
  rc = esp_pm_configure(&pm_config);
  while (1)
  {
    uint32_t cpu_freq = esp_clk_cpu_freq();
    ESP_LOGI(MAIN_TAG, "CPU Freq: %lu", cpu_freq);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}