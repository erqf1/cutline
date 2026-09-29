#!/bin/sh
# Installs Cutline for the current user (~/.local). Needs Qt 6 (Widgets, Multimedia) from your distribution.
set -e
DIR="$(cd "$(dirname "$0")" && pwd)"
PREFIX="${HOME}/.local"
mkdir -p "$PREFIX/opt/cutline" "$PREFIX/bin" "$PREFIX/share/applications" "$PREFIX/share/icons/hicolor/256x256/apps"
cp "$DIR/cutline" "$DIR/ffmpeg" "$PREFIX/opt/cutline/"
ln -sf "$PREFIX/opt/cutline/cutline" "$PREFIX/bin/cutline"
cp "$DIR/icon.png" "$PREFIX/share/icons/hicolor/256x256/apps/cutline.png"
sed "s|^Exec=.*|Exec=$PREFIX/opt/cutline/cutline %f|" "$DIR/cutline.desktop" > "$PREFIX/share/applications/cutline.desktop"
echo "Cutline installed. Run: cutline"
