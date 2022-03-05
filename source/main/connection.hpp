#pragma once

#include <iostream>
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include <esp_https_ota.h>
#include <esp_log.h>
#include "esp_event_loop.h"
#include <sstream>
#include <cstring>
#include <vector>
#include <esp_http_server.h>
#include "pca9685.hpp"

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

class http_server{
  private:
    static inline http_server * instance = nullptr;
    httpd_handle_t server;
    std::vector<httpd_uri_t> handles;
    queue<servo_command, 20> * command_queue;

    static esp_err_t POST_handler(httpd_req_t * req);
    static esp_err_t GET_handler(httpd_req_t * req);

    http_server();

  public:
    static http_server * get_instance();

    void start_webserver();
    void add_direction_queue(queue<servo_command, 20> * queue_to_use);

    bool has_direction_queue();
    void add_to_direction_queue(servo_command data);

};