# KH Notice Board

A simple and easy-to-setup digital notice board system for Kingdom Halls. Display PDF, JPEG, and PNG files on a screen with minimal configuration. Designed to work with any folder-sync service like Dropbox, Google Drive, iCloud or OneDrive. Runs on **macOS** and **Linux** (arm64, armhf, x86_64).

All you need to do to get started is point the app at your synced folder as the input directory; anything added to that folder will be picked up and displayed automatically.

## Download

Download the latest binary for your platform from the [Releases](https://github.com/JmThrCreator/kh-notice-board/releases) page.

| Platform | Binary |
|---|---|
| macOS (Apple Silicon) | `kh-notice-board-macos-arm64` |
| Linux arm64 | `kh-notice-board-linux-arm64` |
| Linux armhf | `kh-notice-board-linux-armhf` |
| Linux x86_64 | `kh-notice-board-linux-x86_64` |

## Usage

1. Download the binary for your platform
2. Make it executable (Linux/macOS):
   ```sh
   chmod +x kh-notice-board-*
   ```
3. Run it:
   ```sh
   ./kh-notice-board-*
   ```

## Building from Source

### macOS

```sh
./build/macos.sh
```

### Linux

Requires [Podman](https://podman.io):

```sh
./build/podman_linux.sh
```

## License

MIT
