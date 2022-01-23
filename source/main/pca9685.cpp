#include "pca9685.hpp"
#include <algorithm> 
#include <array>

uint16_t pca9685::degrees_to_us(const uint8_t degrees) const{
  return (MIN_WIDTH + (((MAX_WIDTH - MIN_WIDTH) * (degrees)) / (MAX_DEGREE)));
}

uint16_t pca9685::degrees_to_pwm(const uint8_t degrees) const{
  float percentage = float(degrees) / 181.0;
  return float(percentage * float(520 - 83)) + 83;
}

void pca9685::reset(){
  driver->write_byte(address, MODE_1, MODE_1_RESTART);
  delay(10);
}

esp_err_t pca9685::setting_sleep_mode(bool sleep_or_wake_up){
  uint8_t old_value = driver->read_byte(address, MODE_1);
  uint8_t new_value =(old_value & ~MODE_1_SLEEP) | ((sleep_or_wake_up) ? MODE_1_SLEEP : 0);
  driver->write_byte(address, MODE_1, new_value);
  if(!sleep_or_wake_up){
    delay(1);
  }
  return ESP_OK;
}

pca9685::pca9685(uint8_t addr, queue<servo_command, 20> * queue_to_use):
  task("driver"),
  address(addr),
  driver(i2c::get_instance()),
  command_queue(queue_to_use)
{
  uint8_t auto_increment = driver->read_byte(address, MODE_1);
  auto_increment = (auto_increment & ~MODE_1_AI) | MODE_1_AI;
  driver->write_byte(address, MODE_1, auto_increment);
  reset();
  set_pwm_frequency(49); //Looking with the logic analyzer this is closer to 50 then the parameter
}

static inline uint32_t round_div(uint32_t x, uint32_t y){
  return (x + y / 2 ) / y;
}

void pca9685::set_pwm_frequency(uint16_t frequency){
  uint32_t prescaler = round_div(FREQUENCY_OSCILLATOR, (uint32_t)4096 * frequency) - 1;
  prescaler = std::clamp(prescaler, (uint32_t)PRESCALE_MIN, (uint32_t)PRESCALE_MAX);
  setting_sleep_mode(true);
  driver->write_byte(address, PRESCALE, prescaler);
  setting_sleep_mode(false);
}

void pca9685::set_pwm(uint8_t type,  uint16_t cycle_begin, uint16_t cycle_end ){
  driver->write_byte(address, LED0_ON_L + (4 * type), cycle_begin);
  driver->write_byte(address, LED0_ON_L + (4 * type) + 1, cycle_begin >> 8);
  driver->write_byte(address, LED0_ON_L + (4 * type) + 2, cycle_end);
  driver->write_byte(address, LED0_ON_L + (4 * type) + 3, cycle_end >> 8);
}

void pca9685::set_degrees(uint8_t type, uint8_t degrees){
  //saved_values[type] = degrees;
  set_pwm(type, 0, degrees_to_pwm(degrees)); 
}

uint8_t pca9685::get_pwm(uint8_t type){
  return driver->read_byte(address, LED0_ON_L + (4 * type));
}

void pca9685::run(void * user_data){
  servo_command input;
  for(;;){
    if(!command_queue->empty()){
      if(command_queue->pop(input, TickType_t(100))){
        std::cout << std::endl <<  "Added packet to queue: " << input << "\n";
        set_degrees(input.servo_number, input.degrees);
      }
    }
  }
}

uint32_t jip_custom_board::mcpwm_degrees_to_width(const uint16_t degrees){
  return ((MIN_WIDTH) + (((MAX_WIDTH - MIN_WIDTH) * degrees) / 180));
}

bool jip_custom_board::mcpwm_set_unit_pwm(const uint8_t slot, const uint32_t degrees){
  if(slot == 16){
    return (mcpwm_set_duty_in_us(unit16.unit, unit16.timer, MCPWM_OPR_A, mcpwm_degrees_to_width(degrees)) == ESP_OK) ?
            true : false;
  }else if(slot == 17){
    return (mcpwm_set_duty_in_us(unit17.unit, unit17.timer, MCPWM_OPR_A, mcpwm_degrees_to_width(degrees)) == ESP_OK) ?
            true : false;
  }else{
    return false;
  }
}

void jip_custom_board::idle_stance(){
  std::array<uint8_t, 12> temp = {130, 90, 100, 120, 80, 20, 90, 10, 10, 90, 80, 50};
  uint8_t index = 4;
  for(auto element : temp){
    driver.set_degrees(index, element);
    delay(50);
    index++;
  }

  mcpwm_set_unit_pwm(16, 90);
  delay(50);
  mcpwm_set_unit_pwm(17, 140);
  delay(50);

  driver.set_degrees(0, 70);
  delay(50);
  driver.set_degrees(1, 120);
  delay(50);
  driver.set_degrees(2, 100);
  delay(50);
  driver.set_degrees(3, 20);
}

/*
  Setup; 
  Left: LW=0, LE=1, LS=17, LHZ=4, LHY=5, LT=6, LK=7, LC=8, LA=9
  Right: RW=2 , RE=3, RS=16, RHZ=10, RHY=11, RT=12, RK=13, RC=14, RA=15

*/


jip_custom_board::jip_custom_board(queue<servo_command, 20> * queue_to_use):
  driver(0x40, queue_to_use),
  command_queue(queue_to_use),
  unit16({MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM0A, GPIO_NUM_14}),
  unit17({MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM0B, GPIO_NUM_15})
{
  mcpwm_gpio_init(unit16.unit, unit16.output, unit16.pin);
  mcpwm_gpio_init(unit17.unit, unit17.output, unit17.pin);
  mcpwm_config_t pwm_config = {50, 0, 0, MCPWM_DUTY_MODE_0, MCPWM_UP_COUNTER};
  mcpwm_init(unit16.unit, unit16.timer, &pwm_config);
  mcpwm_init(unit17.unit, unit17.timer, &pwm_config);
} 


void jip_custom_board::run(void * user_data){
  servo_command input;
  idle_stance();
  for(;;){
    if(!command_queue->empty()){
      if(command_queue->pop(input, TickType_t(100))){
        std::cout <<  "Added packet to queue: " << input << "\n";
        if(input.servo_number > 15){
          mcpwm_set_unit_pwm(input.servo_number, input.degrees);
        }else{
          driver.set_degrees(input.servo_number, input.degrees);
        }
      }
    }
    delay(200);
  }
}