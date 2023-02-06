#!/bin/bash

source venv/bin/activate
waitress-serve --host 127.0.0.1 app:app