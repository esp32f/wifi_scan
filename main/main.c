#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include "macros.h"


static esp_err_t nvs_init() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ERET( nvs_flash_erase() );
    ERET( nvs_flash_init() );
  }
  ERET( ret );
  return ESP_OK;
}


static void on_wifi(void* arg, esp_event_base_t base, int32_t id, void* data) {
  if (id != WIFI_EVENT_SCAN_DONE) return;
  printf("- WiFi scan done (event)\n");
  printf("- Get scanned AP records\n");
  static uint16_t count = 32;
  static wifi_ap_record_t records[32];
  ERETV( esp_wifi_scan_get_ap_records(&count, records) );
  for(int i=0; i<count; i++) {
    printf("%d. %s : %d\n", i+1, records[i].ssid, records[i].rssi);
  }
}


static esp_err_t wifi_scan() {
  printf("- Initialize TCP/IP adapter\n");
  tcpip_adapter_init();
  printf("- Create default event loop\n");
  ERET( esp_event_loop_create_default() );
  printf("- Initialize WiFi with default config\n");
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ERET( esp_wifi_init(&cfg) );
  printf("- Register WiFi event handler\n");
  ERET( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi, NULL) );
  printf("- Set WiFi mode as station\n");
  ERET( esp_wifi_set_mode(WIFI_MODE_STA) );
  printf("- Set WiFi configuration\n");
  wifi_config_t wifi_config = {.sta = {
    .ssid = "",
    .password = "",
    .scan_method = WIFI_ALL_CHANNEL_SCAN,
    .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
    .threshold.rssi = -127,
    .threshold.authmode = WIFI_AUTH_OPEN,
  }};
  ERET( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
  printf("- Start WiFi\n");
  ERET( esp_wifi_start() );
  printf("- Start WiFi AP scan\n");
  wifi_scan_config_t sconfig = {
    .ssid = NULL,
    .bssid = NULL,
    .channel = 0,
    .show_hidden = 1,
    .scan_type = WIFI_SCAN_TYPE_ACTIVE,
    .scan_time.active.min = 120,
    .scan_time.active.max = 120
  };
  ERET( esp_wifi_scan_start(&sconfig, 0) );
  return ESP_OK;
}


void app_main() {
  printf("- Initialize NVS\n");
  ESP_ERROR_CHECK( nvs_init() );
  ESP_ERROR_CHECK( wifi_scan() );
}
