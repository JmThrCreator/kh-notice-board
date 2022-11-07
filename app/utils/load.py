import os, shutil, PIL
from shutil import copy2
from pdf2image import convert_from_path
from app.utils.info import source_folder, destination_folder
from app.utils.sort import sort_by_date, sort_by_order
from config import basedir

# copies folders into destination folder

def load_folders():

    # deletes destination folder (static/upload)
    if os.path.exists(destination_folder):
        shutil.rmtree(destination_folder)

    # copies all items from source to destination
    shutil.copytree(source_folder, destination_folder, copy_function=copy2)

    # removes items that are not jpegs or pngs and converts pdfs to jpegs
    for folder in os.listdir(destination_folder):
        for item in os.listdir(os.path.join(destination_folder, folder)):
            if item.endswith(".pdf"):
                convert_pdf(os.path.join(destination_folder, folder, item))
                os.remove(os.path.join(destination_folder, folder, item))
            elif item.endswith(".png") or item.endswith(".jpg"):
                page_name = f"--page_number_1--.jpg"
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
        elif item.endswith(".png") or item.endswith(".jpg"):
            page_name = f"--page_number_1--.jpg"
            os.rename(os.path.join(destination_folder, source_sub_folder, item), os.path.join(destination_folder, source_sub_folder, item.split("/")[-1].split(".")[0] + page_name))
        else:
            os.remove(os.path.join(destination_sub_folder, item))

    if os.path.exists(destination_sub_folder) == False:
        os.mkdir(destination_sub_folder)

# converts pdf to jpeg

def convert_pdf(item):
    poppler_path = os.path.join(basedir, "app", "utils", "modules", "poppler", "Library", "bin")
    converted_file = convert_from_path(item, poppler_path=poppler_path)

    # saves converted file to destination folder
    for count, page in enumerate(converted_file):
        page_name = f"--page_number_{count+1}--.jpg"
        page.save(os.path.join(destination_folder, item.split("/")[-1].split(".")[0] + page_name))

# returns image size

def get_size(item, folder, size=1):
    image = PIL.Image.open(os.path.join(destination_folder, folder, item))
    width, height = image.size
    perimiter = 546.82*size
    if width > height:
        ratio = width/height
        height = (ratio-1)*perimiter
        width = perimiter-width
    elif width < height:
        ratio = height/width
        width = (ratio-1)*perimiter
        height = perimiter-height
    else:
        ratio = 1
        width = perimiter/2
        height = perimiter/2
    return width, height    

# returns pages

def get_pages(folder, text):
    pages = []
    for page in os.listdir(os.path.join(destination_folder, folder)):
        try:
            if page.split("--page_number_")[0] == text:
                size = get_size(page, folder=folder, size=3)
                pages.append({"name":page, "width":size[0], "height":size[1], "text":text})
        except:
            pass
    print(pages)
    return pages

# returns list of items in destination folder

def get_folders():
    return os.listdir(destination_folder)

# returns list of items in folder

def get_items(folder=None, item=None, page="folder", sort_by="name"):

    items = []

    if page == "folder":
        for item in os.listdir(os.path.join(destination_folder, folder)):
            # skip item if it contains a page number that isn't 1
            try:
                page_number = int(item.split("--page_number_")[1][0])
                if page_number == 1:
                    size = get_size(item, folder=folder, size=1)
                    text = item.split("--page_number_1--")[0]
                else:
                   continue
            except:
                size = get_size(item, folder=folder, size=1)
                text = item.split(".jpg")[0]
            items.append({"name":item, "width":size[0], "height":size[1], "text":text})

    elif page == "item":
        try:
            text = item.split("--page_number_1--")[0]
            page_number = int(item.split("--page_number_")[1][0])
            if page_number == 1:
                items = get_pages(folder, text)
        except:
            text = item.split(".jpg")[0]
            size = get_size(item, folder=folder, size=3)
            items.append({"name":item, "width":size[0], "height":size[1], "text":text})    

    # sorts items by date, name, or order
    if sort_by == "date":
        items = sort_by_date(items, folder)
    elif sort_by == "order":
        items = sort_by_order(items)

    return items