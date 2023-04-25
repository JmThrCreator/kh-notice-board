#!/bin/bash

source venv/bin/activate
gunicorn -b localhost:5000 app:app