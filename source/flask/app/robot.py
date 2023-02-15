from app.movement_history import movement, movement_list
from typing import List

class robot_movement:
  def __init__(self, la, ls, lk, lhx, lhy, lb, ra, rs, rk, rhx, rhy, rb):
    self.movements = {"left_ankle": la, "left_shin": ls, "left_knee": lk, "left_hip_x": lhx,
                   "left_hip_y": lhy, "left_body": lb, "right_ankle": ra, "right_shin": rs,
                   "right_knee": rk, "right_hip_x": rhx, "right_hip_y": rhy, "right_body": rb}

  def get_movement_index(self, key):
    return self.movements[key]

class robot_structure:
  def __init__(self, la, ls, lk, lhx, lhy, lb, ra, rs, rk, rhx, rhy, rb):
    self.joints = {"left_ankle": la, "left_shin": ls, "left_knee": lk, "left_hip_x": lhx,
                   "left_hip_y": lhy, "left_body": lb, "right_ankle": ra, "right_shin": rs,
                   "right_knee": rk, "right_hip_x": rhx, "right_hip_y": rhy, "right_body": rb}
    self.movements = {}

  def add_movement(self, name, movement) -> None:
    self.movements[name] = movement
  
  def get_movement(self, name) -> List[movement]:
    returnList = []
    for key in self.joints:
      returnList.append(movement(self.joints.get(key), self.movements[name].get_movement_index(key), 0))
    return returnList

class robot:
  def __init__(self):
    self.structure = robot_structure(15, 14, 13, 12 , 6, 5, 10, 11, 9, 8, 7, 4)
    self.connection = movement_list()
    self.structure.add_movement("idle", robot_movement(55, 75, 75, 25, 100, 75, 60, 90, 35, 50, 120, 90))
    self.animation = {"walking": ["idle", "first_stage", "second_stage", "third_stage"]}

  def send_movement(self, name):
    self.connection.send_list(self.structure.get_movement(name))
  
  def send_animation(self, name):
    pass
