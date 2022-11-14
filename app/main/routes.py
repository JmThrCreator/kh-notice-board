from app.main import bp
from app import db
from flask import render_template, redirect, request, url_for
from app.utils.load import load_folders, load_folder, get_folders, get_items
from app.main.forms import Attendance as AttendanceForm
from app.models import Attendance as AttendanceModel
from datetime import date

# index

@bp.route("/", methods=["GET", "POST"])
@bp.route("/index", methods=["GET", "POST"])
def index():
    folders = get_folders()
    if folders == []:
        load_folders()
        folders = get_folders()
        
    if request.method == "POST":
        # check if folder in request.form
        #check if request.form exists
        if "reset" in request.form:
            load_folders()
            folders = get_folders()
        for folder in folders:
            if folder in request.form:
                return redirect(url_for("main.folder", folder=folder, sort="date_ascending"))
    return render_template("main/index.html", folders = folders)

# folder

@bp.route("/folder", methods=["GET", "POST"])
@bp.route("/folder/<folder>/<sort>", methods=["GET", "POST"])
def folder(folder=None, sort="date_ascending"):

    if request.method == "POST":
        if "back" in request.form: 
            return redirect(url_for("main.index"))
        elif "name_button" in request.form:
            folder = request.form.get("name_button")
            return redirect(url_for("main.folder", folder=folder, sort="name"))
        elif "ascending_date_button" in request.form:
            folder = request.form.get("ascending_date_button")
            return redirect(url_for("main.folder", folder=folder, sort="date_ascending"))
        elif "descending_date_button" in request.form:
            folder = request.form.get("descending_date_button")
            return redirect(url_for("main.folder", folder=folder, sort="date_descending"))
        elif "attendance" in request.form:
            congregation = request.form.get("attendance")
            return redirect(url_for("main.attendance", congregation=congregation))
        elif "reset" in request.form:
            folder = request.form.get("reset")
            load_folder(folder)
            return redirect(url_for("main.folder", folder=folder, sort="date_ascending"))
        elif "item" in request.form:
            path = request.form.get("item")
            folder=path.split("/")[0]
            item=path.split("/")[1]
            return redirect(url_for("main.item", folder=folder, item=item))

    items = get_items(folder=folder, page="folder", sort_by=sort)

    return render_template("main/folder.html", folder = folder, items = items)

# item

@bp.route("/item", methods=["GET", "POST"])
@bp.route("/item/<folder>/<item>", methods=["GET", "POST"])
def item(folder=None, item=None):

    if request.method == "POST":
        if "back" in request.form:
            folder = request.form.get("back")
            return redirect(url_for("main.folder", folder=folder, sort="date_ascending"))
    
    items = get_items(folder=folder, item=item, page="item", sort_by="order")

    return render_template("main/item.html", folder = folder, item = item, items = items)

# attendance

@bp.route("/attendance", methods=["GET", "POST"])
def attendance():

    congregation = request.args.get("congregation")
    form = AttendanceForm()

    if request.method == "POST":
        if "back" in request.form:
            folder = request.form.get("back")
            return redirect(url_for("main.folder", folder=folder, sort="date_ascending"))
        elif "clear_attendance" in request.form:
            folder = request.form.get("clear_attendance")
            db.session.query(AttendanceModel).filter_by(congregation=congregation).delete()
            db.session.commit()
        elif "submit" in request.form and form.validate_on_submit():

            # get and prepare the data for the database
            new_attendance = AttendanceModel(
                attendance_number=int(form.attendance.data),
                congregation=(congregation)
            )
            today = date.today().strftime("%b %d")
            attendance_db = db.session.query(AttendanceModel).filter_by(congregation=congregation, day=today).first()
            
            # replace if it's already in the database
            if attendance_db:
                db.session.delete(attendance_db)

            # submit entry to database
            db.session.add(new_attendance)
            db.session.commit()

    attendance_data = db.session.query(AttendanceModel).filter_by(congregation=congregation).all()
    attendance_data.reverse()

    return render_template("main/attendance.html", form=form, attendance_data=attendance_data, congregation=congregation)