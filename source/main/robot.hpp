#pragma once

#include <vector>
#include <array>
#include <experimental/array>

#include "rtos.hpp"
#include "queue.hpp"
#include "pca9685.hpp"

enum class servos{
  left_wrist, 
  left_elbow, 
  left_shoulder,
  right_wrist, 
  right_elbow, 
  right_shoulder,
  left_hip_z,
  left_hip_y, 
  left_thigh, 
  left_knee, 
  left_calf, 
  left_ankle,
  right_hip_z,
  right_hip_y, 
  right_thigh, 
  right_knee, 
  right_calf, 
  right_ankle
};

struct movement{
  servos type;
  uint16_t degrees;
  unsigned int delay_after_move;
};

struct full_move{
  std::vector<movement> move;
};

class sg90_servo {
  private:
    uint8_t slot;
    uint16_t degrees;
    uint16_t min_value, max_value;
    bool value_changed;
    servos type;

  public:
    sg90_servo(uint8_t slot, uint16_t degrees, uint16_t min_value, uint16_t max_value, servos type);

    //Clipping if at min or max
    void set_degrees(uint16_t _degrees);
    servo_command get_info();

    bool is_value_changed();
    void value_set();
    bool is_type_equal(servos type);

};

class body{ 
  private:
    sg90_servo left_wrist, left_elbow, left_shoulder;
    sg90_servo right_wrist, right_elbow, right_shoulder;

    sg90_servo left_hip_z, left_hip_y, left_thigh, left_knee, left_calf, left_ankle;
    sg90_servo right_hip_z, right_hip_y, right_thigh, right_knee, right_calf, right_ankle;

    std::array<sg90_servo, 18> all_motors;    
    queue<servo_command, 20> * command_queue;

  public:
    body(queue<servo_command, 20> * queue_to_use);

    void do_single_move(movement move);

};

class robot : public task{
  private:
    body full_robot;

    void do_multiple_moves(std::vector<movement> moves);

    void test_arm(bool left);
    void test_leg(bool left);

    std::vector<full_move> known_movements;

  public:
    //Queue could be made in this object, as this is kinda the master controller
    robot(queue<servo_command, 20> * queue_to_use);

    void add_full_move(full_move move);
    void do_full_move(std::string name);

    void run(void * user_data) override;
};