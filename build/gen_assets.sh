#!/bin/bash

ASSET_DIR="assets"
OUT_DIR="gen"

mkdir -p "$OUT_DIR"

find "$ASSET_DIR" -type f -print0 | while IFS= read -r -d '' file; do
    filename=$(basename "$file")
    name="${filename//[^a-zA-Z0-9]/_}"

    xxd -i -n "$name" "$file" > "$OUT_DIR/$name.h"

    echo "generated $OUT_DIR/$name.h"
done
