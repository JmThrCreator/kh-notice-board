CALL venv\Scripts\activate
start chrome.exe --kiosk --app=http://127.0.0.1:5000/
python -m flask run