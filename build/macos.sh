#!/bin/sh
set -e

mkdir -p bin
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "building..."

./build/gen_assets.sh

clang -std=c11 -g -Wall -O2 -pthread -Werror -fsanitize=address \
	-I"$PROJECT_ROOT"/lib -I"$PROJECT_ROOT"/src -I"$PROJECT_ROOT"/gen \
	-I"$PROJECT_ROOT"/lib/mupdf -I"$PROJECT_ROOT"/lib/jsmn -I"$PROJECT_ROOT"/lib/mongoose \
	src/platform/macos.c \
	-L"$PROJECT_ROOT"/lib/macos/mupdf -lmupdf_macos_arm64 -lmupdf-third_macos_arm64 \
	-o bin/kh-notice-board-macos-arm64

echo "complete"
