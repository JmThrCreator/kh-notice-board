# KH Notice Board

An easy to-set-up kingdom hall/congregation notice board that can display PDF, JPEG and PNG files and manage attendance. Made using Flask.

## Windows Setup

1. Install python (you can use the Microsoft Store on Windows).

2. Enter into the notice board directory in the command line.

3. Create a virtual environment and install the dependencies using the following commands:

```
python3 -m venv venv
venv\Scripts\activate
python -m pip install --upgrade pip
pip install -r requirements.txt
```

## Linux Setup

1. Install python and chrome-browser-stable

```
sudo apt install python3
sudo apt install python3-venv
sudo apt install google-chrome-stable
```

2. Enter into the notice board directory in the command line.

3. Create a virtual environment and install the dependencies using the following commands:

```
python3 -m venv venv
source venv/bin/activate
python3 -m pip install --upgrade pip
pip install -r requirements.txt
sudo apt-get install poppler-utils
```

## How to use

1. Begin by changing the source folder path in the config.ini file (as shown by the example). This folder should contain your folder directories with PDF, JPEG and PNG files within them. You can find an example of one in the "example_source" folder. Your source folder can be synced to your perfered cloud storage platform, such as OneDrive or Dropbox, for remote access if needed. The source folder location can then be changed any time in the config file.

2. Run 'run.bat'. The notice board should then open in your default browser.

3. (Optional) Make sure the 'run.bat' file runs on startup in case your system is updated, restarted or shut down.

4. (Optional) Set up the scheduler for your system to regularly refresh the files automatically. You should link the scheduler to the file 'scheduler.bat'.

## Configuration

Once it's been set, if you need to change the filepath to your source folder, open the config.ini file and enter it after "source = ".
