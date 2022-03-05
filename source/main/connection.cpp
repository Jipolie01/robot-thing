#include "connection.hpp"
#include <functional>
#include <string>
#include <algorithm>

using namespace std::placeholders;
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

esp_err_t http_server::POST_handler(httpd_req_t * req){
  char buf[100];
  unsigned int ret, remaining = req->content_len;

  while (remaining > 0) {
    /* Read the data for the request */
    if ((ret = httpd_req_recv(req, buf, std::min(remaining, sizeof(buf)))) <= 0) {
      if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
      /* Retry receiving if timeout occurred */
      continue;
      }
      return ESP_FAIL;
    }

    remaining -= ret;

    std::string buffer(buf);
    uint8_t servo_slot = std::stoi(buffer.substr(buffer.find(':')+1, buffer.find(',')));
    uint8_t degrees = std::stoi(buffer.substr(buffer.find(",")+1));
    if(http_server::get_instance()->has_direction_queue()){
      std::cout << "Added to queue" << servo_command{servo_slot, degrees} << std::endl;
      http_server::get_instance()->add_to_direction_queue(servo_command{servo_slot, degrees});
    }else {
      std::cout << "No queue added!" << std::endl;
    }
  }

  httpd_resp_send_chunk(req, NULL, 0);
  return ESP_OK;
}

esp_err_t http_server::GET_handler(httpd_req_t * req){
  return ESP_OK;
}

http_server::http_server():
  server(nullptr),
  handles(),
  command_queue(nullptr)
{
  handles.push_back(httpd_uri_t{"/post", HTTP_POST, 
                      http_server::POST_handler, this});
  handles.push_back(httpd_uri_t{"/get", HTTP_GET,
                      http_server::GET_handler, this});
}

http_server * http_server::get_instance(){
  if(!instance){
    instance = new http_server();
  }
  return instance;
}

void http_server::start_webserver(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;

  ESP_LOGI("SERVER", "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI("Server", "Registering URI handlers");
        for(auto handle : handles){
          httpd_register_uri_handler(server, &handle);
        }
  }else {
    ESP_LOGI("Server", "Error starting server!");
  }
}

void http_server::add_direction_queue(queue<servo_command, 20> * queue_to_use){
  command_queue = queue_to_use;
}

bool http_server::has_direction_queue(){
  return (command_queue != nullptr);
}

void http_server::add_to_direction_queue(servo_command data){
  if(has_direction_queue()){
    command_queue->add(data, TickType_t(100));
  }
}