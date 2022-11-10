# KH Notice Board

An easy to-set-up notice board that can display PDF, JPEG and PNG files and manage attendance.

## Setup

Create a virtual environment and install the dependencies using the following commands:

```
python3 -m venv venv
venv\Scripts\activate
python -m pip install --upgrade pip
pip install -r requirements.txt
```

## How to use

Begin by changing the source folder path in the config.ini file (as shown by the example). This folder should contain your folder directories with PDF, JPEG and PNG files within them. You can find an example of one in the "example_source" folder. Your source folder can be synced to your perfered cloud storage platform, such as OneDrive or Dropbox, for remote access if needed. The source folder location can then be changed any time in the config file. The notice board should then open in your default browser.

## Configuration

Once it's been set, if you need to change the filepath to your source folder, open the config.ini file and enter it after "source = ".
