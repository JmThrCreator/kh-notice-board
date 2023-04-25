#!/bin/bash

source venv/bin/activate
google-chrome-stable -kiosk -app=http://127.0.0.1:5000/ &
gunicorn -b localhost:5000 app:app