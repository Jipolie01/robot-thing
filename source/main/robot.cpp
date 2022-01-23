#include "robot.hpp"

sg90_servo::sg90_servo(uint8_t slot, uint16_t degrees, uint16_t min_value, uint16_t max_value, servos type):
  slot(slot),
  degrees(degrees),
  min_value(min_value),
  max_value(max_value),
  value_changed(true),
  type(type)
{}

void sg90_servo::set_degrees(uint16_t _degrees){
  degrees = (_degrees < min_value) ? min_value : (_degrees > max_value) ? max_value : _degrees;
  value_changed = true;
}

servo_command sg90_servo::get_info(){
  return {slot, degrees};
}

bool sg90_servo::is_value_changed(){
  return value_changed;
}

void sg90_servo::value_set(){
  value_changed = false;
}

body::body(queue<servo_command, 20> * queue_to_use):
  left_wrist(0, 0, 0, 150, servos::left_wrist),
  left_elbow(1, 0, 0, 140, servos::left_elbow),
  left_shoulder(2, 10, 10, 180, servos::left_shoulder),
  right_wrist(3, 0, 0, 150, servos::right_wrist),
  right_elbow(4, 0, 0, 140, servos::right_elbow),
  right_shoulder(5, 5, 5,180, servos::right_shoulder),
  left_hip_z(6, 20, 20, 140, servos::left_hip_z),
  left_hip_y(7, 0, 0, 100, servos::left_hip_y),
  left_thigh(8, 20, 20, 120, servos::left_thigh),
  left_knee(9, 20, 20, 100, servos::left_knee),
  left_calf(10, 20, 20, 100, servos::left_calf),
  left_ankle(11, 20, 20, 70, servos::left_ankle),
  right_hip_z(12, 20, 20, 120, servos::right_hip_z),
  right_hip_y(13, 0, 0, 100, servos::right_hip_y),
  right_thigh(14, 0, 0, 180, servos::right_thigh),
  right_knee(15, 100, 100, 180, servos::right_knee),
  right_calf(16, 0, 0, 120, servos::right_calf),
  right_ankle(17, 0, 0, 40, servos::right_ankle),
  all_motors(std::experimental::make_array( left_wrist, left_elbow, left_shoulder, 
            right_wrist, right_elbow, right_shoulder, left_hip_z, left_hip_y, left_thigh, 
            left_knee, left_calf, left_ankle, right_hip_z, right_hip_y, left_thigh, right_knee, 
            right_calf, right_ankle)),
  command_queue(queue_to_use)
{}

void body::do_single_move(movement move){
  for(auto element : all_motors){
    if(element.is_type_equal(move.type)){
      element.set_degrees(move.degrees);
      command_queue->add(element.get_info(), TickType_t(100));
      element.value_set();
      return;
    }
  }
}

void robot::do_multiple_moves(std::vector<movement> moves){
  for(auto element : moves){
    full_robot.do_single_move(element);
    delay(element.delay_after_move);
  }
}

robot::robot(queue<servo_command, 20> * queue_to_use):
  full_robot(queue_to_use),
  known_movements()
{}

void robot::add_full_move(full_move move){
  known_movements.push_back(move);
}

void robot::do_full_move(std::string name){
  // for(auto element : known_movements){
  //   if(!element.name.compare(name)){
  //     do_multiple_moves(element.move );
  //     return;
  //   }
  // } TODO:
}
