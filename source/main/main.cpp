/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "pca9685.hpp"
#include "queue.hpp"
#include "base.hpp"
#include "connection.hpp"


#include <string>

extern "C" {

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"

#define UART_INPUT

void app_main(void)
{
    printf("Hello world!\n");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#ifdef UART_INPUT

    auto hello =  new queue<servo_command, 20>;
    auto test = new jip_custom_board(hello);

    auto connection = http_server::get_instance();
    auto wifi = new wifi_driver();


    wifi->wait_for_connection();
    std::cout << "Connected" << std::endl;
    connection->add_direction_queue(hello);
    connection->start_webserver();
    test->start();
#endif                        
    //body->start();
    //input->start();

}

}