from flask import Flask, render_template

app = Flask(__name__)

from app import routes

# cd source/flask
# set FLASK_APP=application
# flask run