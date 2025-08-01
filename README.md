gcc -o cache fs_utils.c cache.c pdf_utils.c `pkpopplerg-config --cflags --libs -glib cairo`
pkg-config --cflags poppler-glib
sudo apt-get install gcc libgl1-mesa-dev xorg-dev

brew install poppler
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:/opt/homebrew/share/pkgconfig:$PKG_CONFIG_PATH"