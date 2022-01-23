#pragma once

#include "rtos.hpp"
#include <iostream>
#include "pca9685.hpp"
#include "queue.hpp"

class command_line_base : public task{
  private:
    std::string prompt;
    queue<servo_command, 20> * queue_to_use;

    command_line_base();

    void init_console();

    void parse_servo(std::string line);
    void parse_movement(std::string line);
  public:
    command_line_base(queue<servo_command, 20> * queue_to_use);

    void run(void * user_data) override;
};