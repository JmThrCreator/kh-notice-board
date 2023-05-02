#!/bin/bash

BASEDIR=$(dirname "$0")
cd $BASEDIR
source venv/bin/activate
gunicorn -b localhost:5000 app:app