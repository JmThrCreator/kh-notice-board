# KH Notice Board

An easy to-set-up notice board that can display PDF and JPEG files and manage attendance.

<img src="https://raw.githubusercontent.com/JmThrCreator/kh-notice-board/master/github/image_1.PNG" width="45%"> <img src="https://raw.githubusercontent.com/JmThrCreator/kh-notice-board/master/github/image_2.PNG" width="45%"> <img src="https://raw.githubusercontent.com/JmThrCreator/kh-notice-board/master/github/image_3.PNG" width="45%"> <img src="https://raw.githubusercontent.com/JmThrCreator/kh-notice-board/master/github/image_4.PNG" width="45%">

## Setup

Create a virtual environment and install the dependencies using the following commands:

```
python3 -m venv venv
venv\Scripts\activate
python -m pip install --upgrade pip
pip install -r requirements.txt
```

## How to use

Begin by running the "run.py" file. The first time it's run, it will request that you select the source folder. This folder should contain your folder directories with PDF and JPEG files within them. You can find an example of one in the "example_source" folder. Your source folder can be synced to your perfered cloud storage platform, such as OneDrive or Dropbox, for remote access if needed. The source folder location can then be changed any time in the config file. The notice board should then open in your default browser.

## Configuration

Once it's been set, if you need to change the filepath to your source folder, open the config.ini file and enter it after "SOURCE = ".

The background can be edited under "app/static/images/background.jpg".
