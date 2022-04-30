from datetime import date
from app import db

class Attendance(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    attendance_number = db.Column(db.Integer)
    congregation = db.Column(db.String(128))
    day = db.Column(db.String(16), index=True, default=date.today().strftime("%b %d"))
    weekday = db.Column(db.String(4), default=date.today().strftime("%A"))
    def __repr__(self):
        return f"{self.congregation}, {self.attendance_number}, {self.day}, {self.weekday}"