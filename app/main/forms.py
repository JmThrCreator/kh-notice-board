from flask_wtf import FlaskForm
from wtforms import SubmitField, DecimalField
from wtforms.validators import DataRequired, NumberRange

class Attendance(FlaskForm):
    attendance = DecimalField("Attendance", validators=[DataRequired(), NumberRange(min=1, max=200)])
    submit = SubmitField("")