#!/bin/sh

ASSET_DIR="assets"
OUT_DIR="gen"

mkdir -p "$OUT_DIR"

find "$ASSET_DIR" -type f | while read -r file; do
	filename=$(basename "$file")
	name=$(echo "$filename" | tr -c 'a-zA-Z0-9' '_')
	xxd -i "$file" | sed "s/unsigned char .*/unsigned char ${name}[] =/" > "$OUT_DIR/$name.h"
	echo "generated $OUT_DIR/$name.h"
done
