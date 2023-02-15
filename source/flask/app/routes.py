from shutil import move
from app import app
from flask import render_template, request
from app.movement_history import movement_list, movement
from app.robot import robot

__list = movement_list()
__robot = robot()

# res = requests.post('http://192.168.178.248:80/post', data=f"M:{servo_number},{movement_degrees}", headers=headers, timeout=5)

@app.route('/')
@app.route('/index', methods =["GET", "POST"])
def index():
  if request.method == "POST":
    if 'send' in request.form.keys() and request.form['send'] == 'Add command to queue robot':
      servo_number = request.form.get("servo")
      movement_degrees = request.form.get("movement")
      delay = request.form.get("delay")
      __list.add(f"Movement: {servo_number}:{movement_degrees}, Delay: {delay}\n",movement(servo_number, movement_degrees, delay))
      print(__list)
    if 'all_send' in request.form.keys() and request.form['all_send'] == 'Send all commands to robot':
      __list.send_all()
    if 'idle' in request.form.keys() and request.form['idle'] == 'Idle':
      __robot.send_movement("idle")
      
  return render_template('index.html', string_list=__list.full_list_strings)