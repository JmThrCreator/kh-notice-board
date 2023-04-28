import os, shutil, PIL
from shutil import copy2
from pdf2image import convert_from_path
from app.utils.info import source_folder, destination_folder
from app.utils.sort import sort_by_date, sort_by_order
from config import basedir
from sys import platform

SUPPORTED_IMAGE_FILES = (".jpg", ".jpeg", ".png")

def load_folders():

    # Refresh destination folder

    if os.path.exists(destination_folder):
        shutil.rmtree(destination_folder)
    else:
        os.mkdir(destination_folder)

    # Copy items

    shutil.copytree(
        source_folder, destination_folder, copy_function=copy2)
    
    # Parse source folder

    for folder in os.listdir(destination_folder):

        # Delete files in the top directory

        if not os.path.isdir(os.path.join(destination_folder, folder)):
            os.remove(os.path.join(destination_folder, folder))
            continue
        
        # Load files

        load_files(folder)


def load_folder(folder):

    destination_sub_folder = os.path.join(destination_folder, folder)
    source_sub_folder = os.path.join(source_folder, folder)

    # Refresh destination sub-folder

    if os.path.exists(destination_sub_folder):
        shutil.rmtree(destination_sub_folder)

    # Copy items

    shutil.copytree(source_sub_folder, destination_sub_folder, copy_function=copy2)

    # Load files

    load_files(destination_sub_folder)

def load_files(folder):

    for item in os.listdir(os.path.join(destination_folder, folder)):

        item_path = (os.path.join(destination_folder, folder, item))
        item_name = os.path.splitext(item_path)[0]
        file_extension = os.path.splitext(item_path)[1]

        if file_extension == ".pdf":
            convert_pdf(item_path, item_name, folder)
            os.remove(item_path)

        elif file_extension in SUPPORTED_IMAGE_FILES:
            item_name = f"{item_name}--page_number_1--{file_extension}"
            os.rename(
                item_path, os.path.join(destination_folder, folder, item_name))
            
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

        page.save(os.path.join(destination_folder, folder, new_item_name))

def get_width(item, folder, size=1):
    
    image = PIL.Image.open(os.path.join(destination_folder, folder, item))
    width, height = image.size
    perimeter = 546.82*size

    if width > height:
        height = (width / height - 1) * perimeter
        width = perimeter - height
    elif width < height:
        width = (height / width - 1) * perimeter
        height = perimeter - width
    else:
        width = perimeter / 2

    return width
                               
def get_pages(folder, text):

    pages = []

    for page in os.listdir(os.path.join(destination_folder, folder)):
        try:
            if page.split("--page_number_")[0] == text:
                width = get_width(
                    page, folder=folder, size=3)
                
                pages.append(
                    {"name": page, "width": width, "text": text})
        except:
            continue
    return pages

def get_folders():

    if not os.path.exists(destination_folder):
        os.mkdir(destination_folder)
    return os.listdir(destination_folder)

def is_page_multiple(item, folder):

    count = 0
    for item_check in os.listdir(os.path.join(destination_folder, folder)):
        if item_check.split("--page_number_")[0] == item.split("--page_number_")[0]:
            count += 1
            
    if count > 1: 
        return True
    return False

def get_items(folder=None, item=None, page="folder", sort_by="name"):

    items = []

    if page == "folder":

        for item in os.listdir(os.path.join(destination_folder, folder)):

            if os.path.isdir(item) or not item.endswith(SUPPORTED_IMAGE_FILES):
                continue

            # Skip item if it isn't the first page

            page_number = int(item.split("--page_number_")[1][0])

            if page_number != 1:
                continue

            width = get_width(item, folder=folder, size=1)
            text = item.split("--page_number_1--")[0]
            multiple = is_page_multiple(item=item, folder=folder)

            items.append({"name":item, "width":width, "text":text, "multiple":multiple})

    elif page == "item":


        text = item.split("--page_number_1--")[0]
        page_number = int(item.split("--page_number_")[1][0])
        
        if page_number == 1:
            items = get_pages(folder, text)

    # Item sort

    SORT_FUNCTIONS = {
        "date_ascending": sort_by_date(items, folder),
        "date_descending": sort_by_date(items, folder, reverse=True),
        "order": sort_by_order(items)
    }

    if sort_by in SORT_FUNCTIONS:
        items = SORT_FUNCTIONS[sort_by]

    return items