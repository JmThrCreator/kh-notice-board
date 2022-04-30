from flask import Flask
from config import Config as config_class
from flask_sqlalchemy import SQLAlchemy
from flask_migrate import Migrate
from flask_admin import Admin
from flask_admin.contrib.sqla import ModelView

# create app

app = Flask(__name__)

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