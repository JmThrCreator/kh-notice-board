CALL venv\Scripts\activate
start chrome.exe --kiosk --app=http://127.0.0.1:5000/
waitress-serve --port=5000 app:app