#include "connection.hpp"

EventGroupHandle_t wifi_driver::events;

esp_err_t wifi_driver::wifi_handler(void *data, system_event_t *event){
  switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_driver::events, BIT0);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_driver::events, BIT0);
        break;
    default:
        break;
    }
    return ESP_OK;
}

wifi_driver::wifi_driver(){
  tcpip_adapter_init();
  wifi_driver::events = xEventGroupCreate();
  ESP_ERROR_CHECK( esp_event_loop_init(wifi_driver::wifi_handler, NULL) );
  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&config) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  wifi_config_t wifi_config = {.sta = {}};
  memcpy(wifi_config.sta.ssid, WIFI_NAME, sizeof(WIFI_NAME));
  memcpy(wifi_config.sta.password, PASSWORD_NAME, sizeof(PASSWORD_NAME));
  wifi_config.sta.scan_method = WIFI_FAST_SCAN;
  wifi_config.sta.bssid_set = false;

  std::cout << "Setting WiFi configuration SSID " <<  wifi_config.sta.ssid << std::endl;
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
  ESP_ERROR_CHECK( esp_wifi_start() );
}

void wifi_driver::wait_for_connection(){
  xEventGroupWaitBits(wifi_driver::events, BIT0, false, true, portMAX_DELAY);
}

esp_err_t data_sending::event_handler(esp_http_client_event_t * event){
  switch (event->event_id){
    case HTTP_EVENT_ERROR:
      ESP_LOGD(SERVER_TAG, "HTTP_EVENT_ERROR");
      break;
    case HTTP_EVENT_ON_CONNECTED:
      ESP_LOGD(SERVER_TAG, "HTTP_EVENT_ON_CONNECTED");
      break;
    case HTTP_EVENT_HEADER_SENT:
      ESP_LOGD(SERVER_TAG, "HTTP_EVENT_HEADER_SENT");
      break;
    case HTTP_EVENT_ON_HEADER:
      ESP_LOGD(SERVER_TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", event->header_key,
        event->header_value);
      break;
    case HTTP_EVENT_ON_DATA:
      ESP_LOGD(SERVER_TAG, "HTTP_EVENT_ON_DATA, len=%d", event->data_len);
      break;
    case HTTP_EVENT_ON_FINISH:
      ESP_LOGD(SERVER_TAG, "HTTP_EVENT_ON_FINISH");
      break;
    case HTTP_EVENT_DISCONNECTED:
      ESP_LOGD(SERVER_TAG, "HTTP_EVENT_DISCONNECTED");
      break;
  }
  return ESP_FAIL;
}

data_sending::data_sending():
  task(SERVER_TAG),
  client(nullptr),
  wifi()
{
}

void data_sending::connect_to_server(std::string server_url){
  esp_http_client_config_t config = {};

  config.url = server_url.c_str();
  config.event_handler = data_sending::event_handler;

  client = esp_http_client_init(&config);
  esp_http_client_set_url(client, server_url.c_str());
}

void data_sending::send_POST_request(std::string data){

  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_method(client, HTTP_METHOD_POST);
  esp_http_client_set_post_field(client, data.c_str(), data.size());
  esp_err_t error = esp_http_client_perform(client);
  if(error == ESP_OK){
    ESP_LOGI(SERVER_TAG, "HTTP POST Status = %d, content_length = %d",
            esp_http_client_get_status_code(client),
            esp_http_client_get_content_length(client));
  } else {
    ESP_LOGE(SERVER_TAG, "HTTP POST request failed: %s", esp_err_to_name(error));
  }
}

void data_sending::run(void * data){
  for(;;){
    wifi.wait_for_connection();

    if(!client){
      connect_to_server("http://192.168.2.13:5000/upload_data");
    }else{
      //send_data_point_POST_request(accelerometer.get_data());
    }

    delay(100);

  }
}