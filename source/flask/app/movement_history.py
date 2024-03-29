
from app.sending_messages import http_sending
import time


class movement:
  def __init__(self, servo_number, movement_degrees, after_delay):
    self.number = servo_number
    self.degrees = movement_degrees
    self.delay = after_delay

  def __str__(self):
    return f"[{self.number}, {self.degrees}, {self.delay}]\n"

class movement_list():
  def __init__(self):
    self.full_list_of_commands = []
    self.full_list_strings = []
    self.http_sending = http_sending()
    self.headers = {'Content-type': 'text/html; charset=UTF-8'}
  
  def __str__(self):
    f = ""
    for i in self.full_list_of_commands:
      f += i.__str__()
    return f

  def add(self, key,  point : movement):
    self.full_list_strings.append(key)
    self.full_list_of_commands.append(point)

  def send_all(self):
    for i in self.full_list_of_commands:
      self.http_sending.send('http://192.168.178.249:80/post', f"M:{i.number},{i.degrees}", self.headers)
      seconds = 0.001 * float(i.delay)
      time.sleep(seconds)
    self.full_list_of_commands = []
    self.full_list_strings = []

  def send_list(self, list_of_movements):
    for i in list_of_movements:
        self.http_sending.send('http://192.168.178.249:80/post', f"M:{i.number},{i.degrees}", self.headers)