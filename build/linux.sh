#!/bin/sh
set -e
mkdir -p bin
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
echo "building..."
"$PROJECT_ROOT"/build/gen_assets.sh

# Linux x86_64
clang -std=c11 -g -Wall -O2 -pthread -Werror \
	--target=x86_64-linux-gnu \
	-I"$PROJECT_ROOT"/lib -I"$PROJECT_ROOT"/src -I"$PROJECT_ROOT"/gen \
	-I"$PROJECT_ROOT"/lib/mupdf -I"$PROJECT_ROOT"/lib/jsmn -I"$PROJECT_ROOT"/lib/mongoose \
	src/platform/linux.c \
	-L"$PROJECT_ROOT"/lib/linux/mupdf -lmupdf_linux_x86_64 -lmupdf-third_linux_x86_64 \
	-o bin/kh-notice-board-linux-x86_64 -lm

# Linux arm64
clang -std=c11 -g -Wall -O2 -pthread -Werror \
	--target=aarch64-linux-gnu \
	-I"$PROJECT_ROOT"/lib -I"$PROJECT_ROOT"/src -I"$PROJECT_ROOT"/gen \
	-I"$PROJECT_ROOT"/lib/mupdf -I"$PROJECT_ROOT"/lib/jsmn -I"$PROJECT_ROOT"/lib/mongoose \
	src/platform/linux.c \
	-L"$PROJECT_ROOT"/lib/linux/mupdf -lmupdf_linux_arm64 -lmupdf-third_linux_arm64 \
	-o bin/kh-notice-board-linux-arm64 -lm

# Linux armhf
clang -std=c11 -g -Wall -O2 -pthread -Werror \
	--target=armv7-linux-gnueabihf \
	-fuse-ld=/usr/bin/arm-linux-gnueabihf-ld \
	-I/usr/arm-linux-gnueabihf/include \
	-I/usr/include/arm-linux-gnueabihf \
	-L/usr/arm-linux-gnueabihf/lib \
	-I"$PROJECT_ROOT"/lib -I"$PROJECT_ROOT"/src -I"$PROJECT_ROOT"/gen \
	-I"$PROJECT_ROOT"/lib/mupdf -I"$PROJECT_ROOT"/lib/jsmn -I"$PROJECT_ROOT"/lib/mongoose \
	src/platform/linux.c \
	-L"$PROJECT_ROOT"/lib/linux/mupdf -lmupdf_linux_armhf -lmupdf-third_linux_armhf \
	-o bin/kh-notice-board-linux-armhf -lm

echo "complete"
