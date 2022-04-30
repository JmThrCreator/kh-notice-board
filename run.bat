cd .
CALL venv\Scripts\activate
python run.py
start "" http://localhost:5000/
python -m flask run