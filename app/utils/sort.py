import os

# sorts list of files by date

def sort_by_date(items, folder, destination_folder):
    items = [(item, os.path.getmtime(os.path.join(destination_folder, folder, item.get("name")))) for item in items]
    items = sorted(items, key=lambda x: x[1])
    return [item[0] for item in items]

# sorts list of files by name

def sort_by_name(items):
    return sorted(items, key=lambda x: os.path.basename(x.get("name")))

# sorts list of files by order

def sort_by_order(items):
    for item in items:
        try:
            page_number = int(item.get("name").split("--")[1][-1])
        except:
            return sort_by_name(items)
    else:
        return sorted(items, key=lambda x: int(item.get("name").split("--")[1][-1]))
    