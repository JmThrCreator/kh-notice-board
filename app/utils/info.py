import os
from config import basedir
from dotenv import load_dotenv

load_dotenv()

SOURCE_FOLDER = os.getenv("SOURCE_PATH")
DESTINATION_FOLDER = os.path.join(basedir, "app", "static", "upload", "")