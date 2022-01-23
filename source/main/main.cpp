/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "pca9685.hpp"
#include "queue.hpp"
#include "base.hpp"

#include <string>

extern "C" {

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"

#define UART_INPUT

void app_main(void)
{
    printf("Hello world!\n");

#ifdef UART_INPUT

    auto hello =  new queue<servo_command, 20>;
    auto test = new jip_custom_board(hello);
    auto user = new command_line_base(hello);

    test->start();
    user->start();
#endif                        
    //body->start();
    //input->start();

}

}