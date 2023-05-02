from flask import Flask
from config import Config as config_class
from flask_sqlalchemy import SQLAlchemy
from flask_migrate import Migrate
from flask_admin import Admin
from flask_admin.contrib.sqla import ModelView
import os, platform
from sys import platform
from dotenv import load_dotenv

# create app

app = Flask(__name__)

if platform == "win32":
    if __name__ == '__main__':
        from waitress import serve
        serve(app, host='0.0.0.0', port=8000)

# config

app.config.from_object(config_class)

# set up database

db = SQLAlchemy(app)
migrate = Migrate(app, db)

# blueprints

from app.main import bp as main_bp
app.register_blueprint(main_bp)

from app.errors import bp as errors_bp
app.register_blueprint(errors_bp)

# admin

from app.models import Attendance
from app import db

admin = Admin(app, name='Notice Board', template_mode='bootstrap3')
admin.add_view(ModelView(Attendance, db.session))

from app import models

# runs window if not in developer mode

load_dotenv()
if os.getenv("LOAD_CHROME") == "true":
    if platform == "linux":
        os.system("google-chrome-stable -kiosk -app=http://127.0.0.1:5000/ &")
    elif platform == "win32":
        os.system("start chrome.exe --kiosk --app=http://127.0.0.1:5000/")