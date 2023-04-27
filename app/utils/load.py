import os, shutil, PIL
from shutil import copy2
from pdf2image import convert_from_path
from app.utils.info import source_folder, destination_folder
from app.utils.sort import sort_by_date, sort_by_order
from config import basedir
from sys import platform

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

# loads single folder

def load_folder(folder):

    destination_sub_folder = os.path.join(destination_folder, folder)
    source_sub_folder = os.path.join(source_folder, folder)

    # deletes destination folder (static/upload/{sub_folder})
    if os.path.exists(destination_sub_folder):
        shutil.rmtree(destination_sub_folder)

    # copies all items from source to destination
    shutil.copytree(source_sub_folder, destination_sub_folder, copy_function=copy2)

    # removes items that are not jpegs or pngs and converts pdfs to jpegs
    for item in os.listdir(os.path.join(destination_sub_folder)):
        if item.endswith(".pdf"):
            convert_pdf(os.path.join(destination_sub_folder, item))
            os.remove(os.path.join(destination_sub_folder, item))
        elif item.endswith(".jpg") or item.endswith(".jpeg"):
            page_name = f"--page_number_1--.jpg"
            os.rename(os.path.join(destination_sub_folder, item), os.path.join(destination_sub_folder, item.split("/")[-1].split(".")[0] + page_name))
        elif item.endswith(".png"):
            page_name = f"--page_number_1--.png"
            os.rename(os.path.join(destination_sub_folder, item), os.path.join(destination_sub_folder, item.split("/")[-1].split(".")[0] + page_name))
        else:
            os.remove(os.path.join(destination_sub_folder, item))

    if os.path.exists(destination_sub_folder) == False:
        os.mkdir(destination_sub_folder)

# converts pdf to jpeg

def convert_pdf(item):
    if platform == "linux":
        converted_file = convert_from_path(item)
    elif platform == "win32":
        poppler_path = os.path.join(basedir, "app", "utils", "modules", "poppler", "Library", "bin")
        converted_file = convert_from_path(item, poppler_path=poppler_path)

    # saves converted file to destination folder
    for count, page in enumerate(converted_file):
        page_name = f"--page_number_{count+1}--.jpg"
        if platform == "linux":
            page.save(os.path.join(destination_folder, item + page_name))
        elif platform == "win32":
            page.save(os.path.join(destination_folder, item.split("/")[-1].split(".")[0] + page_name))

# returns image width

def get_width(item, folder, size=1):
    image = PIL.Image.open(os.path.join(destination_folder, folder, item))
    width, height = image.size
    perimiter = 546.82*size
    if width > height:
        ratio = width/height
        height = (ratio-1)*perimiter
        width = perimiter-height
    elif width < height:
        ratio = height/width
        width = (ratio-1)*perimiter
    else:
        width = perimiter/2
    return width

# returns pages

def get_pages(folder, text):
    pages = []
    for page in os.listdir(os.path.join(destination_folder, folder)):
        try:
            if page.split("--page_number_")[0] == text:
                width = get_width(page, folder=folder, size=3)
                pages.append({"name":page, "width":width, "text":text})
        except:
            pass
    return pages

# returns list of items in destination folder

def get_folders():
    if not os.path.exists(destination_folder):
        os.mkdir(destination_folder)
    return os.listdir(destination_folder)

# returns True if item has multiple pages

def is_page_multiple(item, folder):
    count = 0
    for item_check in os.listdir(os.path.join(destination_folder, folder)):
        if item_check.split("--page_number_")[0] == item.split("--page_number_")[0]:
            count += 1
    if count > 1: return True
    return False

# returns list of items in folder

def get_items(folder=None, item=None, page="folder", sort_by="name"):

    items = []

    if page == "folder":
        for item in os.listdir(os.path.join(destination_folder, folder)):
            if os.path.isdir(item):
                continue
            elif not (item.endswith(".png") or item.endswith(".jpg") or item.endswith(".jpeg")):
                continue
            # skip item if it contains a page number that isn't 1
            multiple = False
            try:
                page_number = int(item.split("--page_number_")[1][0])
                if page_number == 1:
                    width = get_width(item, folder=folder, size=1)
                    text = item.split("--page_number_1--")[0]
                    multiple = is_page_multiple(item=item, folder=folder)
                else:
                   continue
            except:
                width = get_width(item, folder=folder, size=1)
                if item.endswith(".png"):
                    text = item.split(".png")[0]
                elif item.endswith(".jpg"):
                    text = item.split(".jpg")[0]
                else:
                    text = item.split(".jpeg")[0]
            items.append({"name":item, "width":width, "text":text, "multiple":multiple})

    elif page == "item":
        try:
            text = item.split("--page_number_1--")[0]
            page_number = int(item.split("--page_number_")[1][0])
            if page_number == 1:
                items = get_pages(folder, text)
        except:
            if item.endswith(".png"):
                text = item.split(".png")[0]
            else:
                text = item.split(".jpg")[0]
            width = get_width(item, folder=folder, size=3)
            items.append({"name":item, "width":width, "text":text})    

    # sorts items by date, name, or order
    if sort_by == "date_ascending":
        items = sort_by_date(items, folder)
        items.reverse()
    if sort_by == "date_descending":
        items = sort_by_date(items, folder)
    elif sort_by == "order":
        items = sort_by_order(items)

    return items
