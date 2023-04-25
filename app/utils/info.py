import os, configparser
from config import basedir

# get data from config.ini

def get_config():
    config = configparser.ConfigParser()
    config_file_path = os.path.join(basedir, "config.ini")
    config.read(config_file_path)
    return config

# get source folder

config = get_config()
source_folder = config.get("source file location", "source")

# get destination folder

destination_folder = os.path.join(basedir, "app", "static", "upload", "")
