import os
from flask_sqlalchemy import SQLAlchemy
from dotenv import load_dotenv

basedir = os.path.abspath(os.path.dirname(__file__))

load_dotenv()
DATABASE_PATH = os.getenv("DATABASE_PATH", default=basedir)

# create config class

class Config(object):
    SECRET_KEY = os.environ.get("SECRET_KEY") or "secret"
    SQLALCHEMY_DATABASE_URI = os.environ.get("DATABASE_URL") or \
        "sqlite:///" + os.path.join(DATABASE_PATH, "app.db")
    SQLALCHEMY_TRACK_MODIFICATIONS = False
    FLASK_ADMIN_SWATCH = 'cerulean'

