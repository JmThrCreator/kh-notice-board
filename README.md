# KH Notice Board

Easy to set-up and customisable notice board for kingdom halls and congregations that displays PDF, JPEG, and PNG files. Made using Flask.

## Installation

### Linux

Open the command prompt and enter the following commands:

- Install Git: `sudo apt-get install git`
- Install notice board app: `git clone https://github.com/JmThrCreator/kh-notice-board.git`
- Navigate to the program: `cd kh-notice-board`
- Install Python:
```
sudo apt install python3
sudo apt install python3-venv
sudo apt install python3-pip
```
- Install chrome (and delete installer):
```
wget https://dl.google.com/linux/direct/google-chrome- stable_current_amd64.deb
sudo dpkg -i google-chrome-stable_current_amd64.deb
sudo rm -r google-chrome-stable_current_amd64.deb
```
- Install required python libraries:
```
python3 -m venv venv
source venv/bin/activate
python -m pip install --upgrade pip
pip install -r requirements.txt
```

### Windows

- Install Git: https://git-scm.com/download/win
- Install Python: https://www.microsoft.com/store/productId/9NRWMJP3717K

Open the command prompt and enter the following commands:
- Install notice board app: `git clone https://github.com/JmThrCreator/kh-notice-board.git`
- Navigate to the program: `cd kh-notice-board`
- Install required python libraries:
```
python3 -m venv venv
venv\Scripts\activate
python -m pip install --upgrade pip
pip install -r requirements.txt
```

## Usage

### Environment variables

Create a file called .env (On Windows, enable 'File name extensions' in the file explorer under the 'View' tab before doing so).

Add your source folder location to the .env file. This folder should contain each congregation's folder with PDF, JPEG and PNG files within them. You can find an example of one in the "example_source" folder. Your source folder can be synced to your perfered cloud storage platform, such as OneDrive or Dropbox, for remote access if needed.

If you'd like the app to load when you execute the command, set 'LOAD_CHROME' to true. Otherwise, opening any web browser and going to: 'http://localhost:5000/' will open the app.

By default, the program will store your attendance data inside the program folder. However, if you update the program, you will lose all your data. To ensure this doesn't happen, you should store the database outside the program. You can link the the database directory using 'DATABASE_PATH'.

Below is a sample .env file:
```
SOURCE_PATH = path/to/kh-notice-board/example_source
LOAD_CHROME = true
DATABASE_PATH = path/to/database
```

### Running the program

Run 'run.bat' (Windows) or 'run.sh' (Linux). The notice board should then open in your default browser.

### Recommended extras

Running the program on startup is useful in case your system is updated, restarted or shut down.

If you want to set up a scheduler for your system to regularly refresh the files, you can link the scheduler to the file 'scheduler.bat'.