#include "rtos.hpp"
#include <esp_log.h>

void task::run_task(void *taskInstance){
  task* temp_task = (task*) taskInstance;
  ESP_LOGI("RTOS", ">> runTask: taskName=%s", temp_task->task_name.c_str());
  temp_task->run(temp_task->task_data);
  ESP_LOGI("RTOS", "<< runTask: taskName=%s", temp_task->task_name.c_str());
  temp_task->stop();
}

task::task(std::string name, uint16_t size, uint8_t task_priority){
  task_name  = name;
  stack_size = size;
  priority   = task_priority;
  task_data  = nullptr;
  handle     = nullptr;
  core_id    = tskNO_AFFINITY;
}

void task::set_name(std::string name){
  task_name = name;
}

void task::set_stack_size(uint16_t size){
  stack_size = size;
}

void task::set_priority(uint8_t taskPriority){
  priority = taskPriority;
}

void task::set_core(BaseType_t id){
  core_id = id;
}

void task::start(void* data){
  if (handle){
    ESP_LOGW(task_name.c_str(), "task::start - There might be a task already running!");
  }

  task_data = data;
  xTaskCreatePinnedToCore(&run_task, task_name.c_str(), stack_size, this, priority, &handle, core_id);
}

void task::stop(void){
  if (!handle){
    return;
  }

  xTaskHandle temp = handle;
  handle = nullptr;
  vTaskDelete(temp);
}

void task::delay(unsigned int ms){
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

void task::delay_until(TickType_t *wake_time, int ms){
  vTaskDelayUntil(wake_time, ms / portTICK_PERIOD_MS);
}

void task::suspend(void){
  vTaskSuspend(handle);
}

void task::resume(void){
  vTaskResume(handle);
}
