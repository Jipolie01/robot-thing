#pragma once
#include <iostream>
#include <optional>
#include <experimental/array>

#include "driver/i2c.h"
#include "rtos.hpp"
#include "queue.hpp"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include <esp_system.h>

#define MIN_WIDTH 1000
#define MAX_WIDTH 2000
#define MAX_DEGREE 255
#define PRESCALE 0xFE
#define LED0_ON_L 0x06
#define FREQUENCY_OSCILLATOR 25000000  /**< Int. osc. frequency in datasheet */
#define PRESCALE_MIN 3
#define PRESCALE_MAX 255
#define MODE_1 0x00
#define MODE_1_RESTART 0x80
#define MODE_1_AI 0x20
#define MODE_1_SLEEP 0x10

struct servo_command{
  uint8_t servo_number;
  uint16_t degrees;
};

inline std::ostream& operator<<(std::ostream & oss, const servo_command & other)
{
   oss << "{ " << (int)other.servo_number << " , " << (int)other.degrees << " }";
   return oss;
}

class i2c{
  private:
    static inline i2c * instance = nullptr;

    i2c(gpio_num_t sda, gpio_num_t scl){
      i2c_config_t config;
      config.mode = I2C_MODE_MASTER;
      config.scl_io_num = scl;
      config.scl_pullup_en = GPIO_PULLUP_ENABLE;
      config.sda_io_num = sda;
      config.sda_pullup_en = GPIO_PULLUP_ENABLE;
      config.master.clk_speed = 100000;

      i2c_param_config(I2C_NUM_0, &config);
      i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    }

  public:

    static i2c* get_instance(){
      if(!instance){
        instance = new i2c(GPIO_NUM_21, GPIO_NUM_22);
      }
      return instance;
    }

    esp_err_t write_byte(uint8_t address, uint8_t reg, uint8_t data){
      i2c_cmd_handle_t command = i2c_cmd_link_create();
      i2c_master_start(command);
      i2c_master_write_byte(command, (address << 1) | I2C_MASTER_WRITE, 0x1);
      i2c_master_write_byte(command, reg, 0x1);
      i2c_master_write(command, &data, 1, 0x1);
      i2c_master_stop(command);
      esp_err_t result = i2c_master_cmd_begin(I2C_NUM_0, command, pdMS_TO_TICKS(1000));
      i2c_cmd_link_delete(command);
      if(result != ESP_OK){std::cout << "Write error!" << std::endl;}
      return result;
    }
    uint8_t read_byte(uint8_t address, uint8_t reg){
      uint8_t data;
      i2c_cmd_handle_t command = i2c_cmd_link_create();
      i2c_master_start(command);
      i2c_master_write_byte(command, (address << 1) | I2C_MASTER_WRITE, 0x1);
      i2c_master_write_byte(command, reg, I2C_MASTER_ACK);
      i2c_master_start(command);
      i2c_master_write_byte(command, address << 1 | I2C_MASTER_READ, 0x1);
      i2c_master_read_byte(command, &data, I2C_MASTER_NACK);
      i2c_master_stop(command);
      esp_err_t result = i2c_master_cmd_begin(I2C_NUM_0, command, 1000 / portTICK_RATE_MS);
      if(result != ESP_OK){std::cout << "Read error!" << std::endl;}
      i2c_cmd_link_delete(command);
      return data;
    }

};

class pca9685 : public task{
  private:
    uint8_t address;
    i2c * driver;
    queue<servo_command, 20> * command_queue;

    uint16_t degrees_to_us(const uint8_t degrees) const;
    uint16_t degrees_to_pwm(const uint8_t degrees) const;
    void reset();

    esp_err_t setting_sleep_mode(bool sleep_or_wake_up);

  public:
    pca9685(uint8_t addr, queue<servo_command, 20> * queue_to_use);

    void set_pwm_frequency(uint16_t frequency);
    void set_us_duty(uint8_t type, uint16_t microseconds);
    void set_pwm(uint8_t type, uint16_t cycle_begin, uint16_t cycle_end);
    void set_degrees(uint8_t type, uint8_t degrees);

    uint8_t get_pwm(uint8_t type);
    void run(void * user_data) override;
};

struct pwm_configurables{
  mcpwm_unit_t unit;
  mcpwm_timer_t timer;
  mcpwm_io_signals_t output;
  gpio_num_t pin;
};

class jip_custom_board : public task {
  private:
    pca9685 driver;
    queue<servo_command, 20> * command_queue;
    pwm_configurables unit16, unit17;

    uint32_t mcpwm_degrees_to_width(const uint16_t degrees);

    bool mcpwm_set_unit_pwm(const uint8_t slot, const uint32_t degrees);
    void idle_stance();

  public:
    jip_custom_board(queue<servo_command, 20> * queue_to_use); //Do pins internally

    void run(void * user_data) override;

};
