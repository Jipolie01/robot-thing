#pragma once

#include <iostream>
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include <esp_https_ota.h>
#include "esp_log.h"
#include "esp_event_loop.h"
#include <sstream>
#include <cstring>

#define WIFI_NAME       "ZiggoC78F229"
#define PASSWORD_NAME   "Nzfn2jukwjxn"

class wifi_driver{
  private:
    static EventGroupHandle_t events;

    static esp_err_t wifi_handler(void *data, system_event_t *event);
  public:
    wifi_driver();

    void wait_for_connection();
};

class data_sending : public task{
  private:
    esp_http_client_handle_t client;
    wifi_driver wifi;

    static esp_err_t event_handler(esp_http_client_event_t * event);

  public:
    data_sending();

    void connect_to_server(std::string server_url);
    void send_POST_request(std::string data);
    std::string receive_request();

    void run(void * data) override;
};