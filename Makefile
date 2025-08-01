CC = gcc
CFLAGS += -Wall -I$(PWD)/include -I$(PWD)/lib $(shell pkg-config --cflags poppler-glib)
LDFLAGS = $(shell pkg-config --libs poppler-glib)

SRC = src/app.c src/cache.c src/fs_utils.c src/pdf_utils.c src/arena.c src/web.c lib/mongoose.c
OUT = bin/kh-notice-board

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)

.PHONY: all clean
