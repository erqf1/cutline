# Cutline

A simple, fast video editor. Open an mp4 or mov, cut it, change the speed of parts, blur faces or text, drop in images, videos and music, and export. Nothing else.

- **Also a good video player.** Make it your default app for mp4/mov/mkv: instant start, fullscreen, click to pause, 5-second skip, ambient light around the video.
- **Opens instantly.** No import step, no project setup. Drag a video in and it plays.
- **Edit mode with a timeline.** Trim, split, remove segments, per-segment speed, image overlays, blur areas (the blur is real, also in the preview), extra videos and a sound track lane.
- **Sensible export.** Pick resolution (4K … SD), frame rate and quality. Only values up to the original are offered, so you never waste disk space.
- **Fullscreen** with `F11`, click the video to pause, arrow keys jump 5 seconds.
- **12 languages** and several themes (including a full Windows XP Luna look).
- Uses your GPU encoder when available (NVENC, Quick Sync, AMF, VideoToolbox), otherwise x264.

## Download

Get the latest build from the [Releases page](../../releases/latest):

| Platform | File |
| --- | --- |
| Windows (x64) installer | `Cutline-windows-x64-setup.exe` – lets you choose the install folder and adds Cutline to the **Open with** menu of videos |
| Windows (x64) portable | `Cutline-windows-x64.zip` – unzip and run `Cutline.exe` |
| Linux (Debian / Ubuntu) | `cutline_amd64.deb` – `sudo apt install ./cutline_amd64.deb` |
| Linux (Arch) | `cutline-x86_64.pkg.tar.zst` – `sudo pacman -U cutline-x86_64.pkg.tar.zst` |
| Linux (any, tar.gz) | `cutline-linux-x86_64.tar.gz` – needs Qt 6 and ffmpeg from your distro |
| macOS Intel | `Cutline-macos-intel.dmg` |
| macOS Apple Silicon | `Cutline-macos-apple-silicon.dmg` |

macOS builds are not notarized. On first start, right-click the app and choose **Open**.

## Publishing (maintainers)

Run `publish.bat`: it asks for your GitHub username, opens the new-repository page, pushes the code and the `v1.0.0` tag (which triggers the release builds) and opens the Pages settings (`main` / `/docs`).

## Keyboard shortcuts

| Key | Action |
| --- | --- |
| `Space` / click video | Play / pause |
| `←` / `→` | Back / forward 5 s |
| `F11`, `Esc` | Fullscreen on / off |
| `E` | Edit mode |
| `S` | Split at playhead |
| `Q` / `W` | Trim start / end to playhead |
| `Del` | Remove selected segment or element |
| `B` / `I` | Add blur area / image |
| `Ctrl+O`, `Ctrl+E` | Open, export |
| `Ctrl+Z`, `Ctrl+Y` | Undo, redo |

## Build from source

Requires Qt 6.4+ (Widgets, Multimedia, MultimediaWidgets), CMake 3.21+ and a C++17 compiler. Cutline calls `ffmpeg` for thumbnails and export; put an `ffmpeg` binary next to the executable or on your `PATH`.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

On Windows, `build.bat` builds and creates the portable zip. Release packages for all platforms are built by GitHub Actions (`.github/workflows/release.yml`) when a `v*` tag is pushed.

## License

MIT for the Cutline source code. Release packages bundle [FFmpeg](https://ffmpeg.org) (GPL build) and [Qt](https://www.qt.io) (LGPLv3), which keep their own licenses.
