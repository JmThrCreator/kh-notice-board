import os, shutil, configparser
from shutil import copy2
from pdf2image import convert_from_path
from sys import platform
from dotenv import load_dotenv

# get data from .env

basedir = os.path.abspath(os.path.dirname(__file__))
basedir = os.path.dirname(basedir)

load_dotenv()
SOURCE_FOLDER = os.get_env("SOURCE_PATH")
DESTINATION_FOLDER = os.path.join(basedir, "app", "static", "upload", "")

SUPPORTED_IMAGE_FILES = (".jpg", ".jpeg", ".png")

def load_folders():

    # Refresh destination folder

    if os.path.exists(DESTINATION_FOLDER):
        shutil.rmtree(DESTINATION_FOLDER)
    else:
        os.mkdir(DESTINATION_FOLDER)

    # Copy items

    shutil.copytree(
        SOURCE_FOLDER, DESTINATION_FOLDER, copy_function=copy2)
    
    # Parse source folder

    for folder in os.listdir(DESTINATION_FOLDER):

        # Delete files in the top directory

        if not os.path.isdir(os.path.join(DESTINATION_FOLDER, folder)):
            os.remove(os.path.join(DESTINATION_FOLDER, folder))
            continue
        
        # Load files

        load_files(folder)

def load_files(folder):

    for item in os.listdir(os.path.join(DESTINATION_FOLDER, folder)):

        item_path = (os.path.join(DESTINATION_FOLDER, folder, item))
        item_name = os.path.splitext(item_path)[0]
        file_extension = os.path.splitext(item_path)[1]

        if file_extension == ".pdf":
            convert_pdf(item_path, item_name, folder)
            os.remove(item_path)

        elif file_extension in SUPPORTED_IMAGE_FILES:
            item_name = f"{item_name}--page_number_1--{file_extension}"
            os.rename(
                item_path, os.path.join(DESTINATION_FOLDER, folder, item_name))
            
        else:
            os.remove(item_path)

def convert_pdf(item_path, item_name, folder):
    
    # Convert PDF to JPG

    if platform == "linux":
        converted_file = convert_from_path(item_path)

    elif platform == "win32":
        poppler_path = os.path.join(basedir, "app", "utils", "modules", "poppler", "Library", "bin")
        converted_file = convert_from_path(item_path, poppler_path=poppler_path)

    # Save file
    
    for count, page in enumerate(converted_file, start=1):
        new_item_name = f"{item_name}--page_number_{count}--.jpg"

        page.save(os.path.join(DESTINATION_FOLDER, folder, new_item_name))

load_folders()
