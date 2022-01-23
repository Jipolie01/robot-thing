#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <iostream>

class task{
  private:
    xTaskHandle handle;
    void*       task_data;
    std::string task_name;
    uint16_t    stack_size;
    uint8_t     priority;
    BaseType_t  core_id;

    static void run_task(void *task_instance);
  public:
    task(std::string name = "Task", uint16_t size = 10000, uint8_t task_priority = 3);
 
    void set_name(std::string name);
    void set_stack_size(uint16_t size);
    void set_priority(uint8_t taskPriority);
    void set_core(BaseType_t id);
    void start(void* data = nullptr);
    void stop(void);
    void delay(unsigned int ms);
    void delay_until(TickType_t *wake_time, int ms);
    void suspend(void);
    void resume(void);

    virtual void run(void* data) = 0;
};