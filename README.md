# Cutline

A simple, fast video editor. Open an mp4 or mov, cut it, change the speed of parts, blur faces or text, drop in images, videos and music, and export. Nothing else.

- **Also a good video player.** Make it your default app for mp4/mov/mkv: instant start, fullscreen, click to pause, 5-second skip, ambient light around the video.
- **Opens instantly.** No import step, no project setup. Drag a video in and it plays.
- **Edit mode with a timeline.** Drag segment edges to trim, split only what is selected (or everything at the playhead), per-segment speed, zoom in/out.
- **Volume like in the big editors.** Waveforms on every clip; drag the volume line up or down and see the level in dB.
- **Resize, move and rotate** each segment, add **text** (bundled display fonts or any installed font, with or without background), image overlays and real blur areas.
- **Media panel.** Everything you import sits on the right; double-click to insert it at the playhead.
- **Sensible export.** MP4, MOV, MKV, GIF or audio only (MP3, WAV, M4A). Pick resolution (4K … SD), frame rate and quality – only values up to the original are offered.
- **Checks for updates** and installs them on request (or ignore a version).
- **Fullscreen** with `F11`, click the video to pause, arrow keys jump 5 seconds.
- **12 languages** and several themes (including a clean "Studio" look and a full Windows XP Luna look).
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

Run `publish.bat`: it asks for your GitHub username, opens the new-repository page, pushes the code and the `v1.0.0` tag (which triggers the release builds).

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

Apache License 2.0 for the Cutline source code. Release packages bundle [FFmpeg](https://ffmpeg.org) (GPL build) and [Qt](https://www.qt.io) (LGPLv3), which keep their own licenses.

Bundled fonts from Google Fonts: Anton, Bangers, Bebas Neue, Creepster, Lobster, Pacifico, Press Start 2P (SIL Open Font License 1.1) and Permanent Marker (Apache License 2.0).
