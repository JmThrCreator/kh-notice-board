import os
from app.utils.info import source_folder, destination_folder

# sorts list of files by date (in source folder)

def sort_by_date(items, folder, reverse=False):
    VALID_EXTENSIONS = (".pdf", ".jpg", ".jpeg", ".png")

    for source_item in os.listdir(os.path.join(source_folder, folder)):

        if not source_item.endswith(VALID_EXTENSIONS):
            continue

        for item in items:
            item_name = item.get("name").split(".")[0].split("--")[0]

            if item_name == source_item.split(".")[0]:
                item["time"] = os.path.getmtime(os.path.join(source_folder, folder, source_item))

    items = sorted(items, key=lambda x: x["time"])

    if reverse:
        items.reverse()
    
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
    