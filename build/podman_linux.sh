#!/bin/sh
set -e
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
if ! podman machine inspect > /dev/null 2>&1; then
    podman machine init
fi
podman machine start 2>/dev/null || true
podman build -t kh-linux-builder -f build/Dockerfile.linux .
podman run --rm \
    -v "$PROJECT_ROOT":/src:z \
    -w /src \
    kh-linux-builder \
    ./build/linux.sh
