import os, shutil, configparser
from shutil import copy2
from pdf2image import convert_from_path
from sys import platform

# get data from config.ini

basedir = os.path.abspath(os.path.dirname(__file__))
basedir = os.path.dirname(basedir)

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


# copies folders into destination folder

def load_folders():

    # deletes destination folder (static/upload)
    if os.path.exists(destination_folder):
        shutil.rmtree(destination_folder)
    else:
        os.mkdir(destination_folder)

    # copies all items from source to destination
    shutil.copytree(source_folder, destination_folder, copy_function=copy2)

    # removes items that are not jpegs or pngs and converts pdfs to jpegs
    for folder in os.listdir(destination_folder):

        # delete item if it's a file
        if not os.path.isdir(os.path.join(destination_folder, folder)):
            os.remove(os.path.join(destination_folder, folder))
            continue

        for item in os.listdir(os.path.join(destination_folder, folder)):
            if item.endswith(".pdf"):
                convert_pdf(os.path.join(destination_folder, folder, item))
                os.remove(os.path.join(destination_folder, folder, item))
            elif item.endswith(".jpg") or item.endswith(".jpeg"):
                page_name = f"--page_number_1--.jpg"
                os.rename(os.path.join(destination_folder, folder, item), os.path.join(destination_folder, folder, item.split("/")[-1].split(".")[0] + page_name))
            elif item.endswith(".png"):
                page_name = f"--page_number_1--.png"
                os.rename(os.path.join(destination_folder, folder, item), os.path.join(destination_folder, folder, item.split("/")[-1].split(".")[0] + page_name))
            else:
                os.remove(os.path.join(destination_folder, folder, item))

    if os.path.exists(destination_folder) == False:
        os.mkdir(destination_folder)

# converts pdf to jpeg

def convert_pdf(item):
    if platform == "linux":
        converted_file = convert_from_path(item)
    elif platform == "win32":
        poppler_path = os.path.join(basedir, "app", "utils", "modules", "poppler", "Library", "bin")
        converted_file = convert_from_path(item, poppler_path=poppler_path)
    else:
        return

    # saves converted file to destination folder
    for count, page in enumerate(converted_file):
        page_name = f"--page_number_{count+1}--.jpg"
        if platform == "linux":
            page.save(os.path.join(destination_folder, item + page_name))
        elif platform == "win32":
            page.save(os.path.join(destination_folder, item.split("/")[-1].split(".")[0] + page_name))

load_folders()
