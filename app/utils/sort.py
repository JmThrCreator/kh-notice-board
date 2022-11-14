import os
from app.utils.info import source_folder, destination_folder

# sorts list of files by date (in source folder)

def sort_by_date(items, folder):

    for source_item in os.listdir(os.path.join(source_folder, folder)):
        if source_item.endswith(".pdf") or source_item.endswith(".jpg") or source_item.endswith(".jpeg") or source_item.endswith(".png"):
            for item in items:
                if item.get("name").split(".")[0].split("--")[0] == source_item.split(".")[0]:
                    item.update({"time":os.path.getmtime(os.path.join(source_folder, folder, source_item))})

    try:
        items = sorted(items, key=lambda x: x["time"])
    except:
        pass
    return items

# sorts list of files by page order

def sort_by_order(items):
    for item in items:
        try:
            page_number = int(item.get("name").split("--")[1][-1])
        except:
            pass
    else:
        return sorted(items, key=lambda x: int(item.get("name").split("--")[1][-1]))
    