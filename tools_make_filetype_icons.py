"""Dateisymbole für Videos und Musik, die mit Cutline geöffnet werden: Blatt mit Eselsohr und Zeilen, das Cutline-Logo
in der Mitte, darunter die Endung (mp3, mp4 ...) und ganz unten ein Streifen - je Typ in eigener Farbe, hell und dunkel.
Welche Variante Windows zeigt, stellt man in Cutline ein (src/fileicons.cpp).
Schreibt assets/filetypes/light|dark/<endung>.ico und packaging/windows/filetypes.iss (Registry-Einträge für den Installer).
Braucht Python 3 + Pillow und Segoe UI (Windows). Aufruf: python tools_make_filetype_icons.py"""
import io
import os
import struct

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(ROOT, 'assets', 'filetypes')
LOGO = Image.open(os.path.join(ROOT, 'assets', 'icon.png')).convert('RGBA')
FONT = next((f for f in ['C:/Windows/Fonts/segoeuib.ttf', 'C:/Windows/Fonts/arialbd.ttf'] if os.path.exists(f)), None)

# Endung -> (Beschriftung, Farbe, Art). "audio"/"video": Sammel-ProgIDs älterer Versionen
TYPES = {
    'mp4': ('mp4', '#1d4ed8', 'video'), 'mov': ('mov', '#0f766e', 'video'), 'mkv': ('mkv', '#7e22ce', 'video'),
    'm4v': ('m4v', '#1e40af', 'video'), 'avi': ('avi', '#b91c1c', 'video'), 'webm': ('webm', '#15803d', 'video'),
    'wmv': ('wmv', '#0369a1', 'video'), 'flv': ('flv', '#c2410c', 'video'),
    'mp3': ('mp3', '#7c3aed', 'audio'), 'wav': ('wav', '#0891b2', 'audio'), 'm4a': ('m4a', '#db2777', 'audio'),
    'aac': ('aac', '#a21caf', 'audio'), 'flac': ('flac', '#059669', 'audio'), 'ogg': ('ogg', '#d97706', 'audio'),
    'opus': ('opus', '#4f46e5', 'audio'), 'wma': ('wma', '#2563eb', 'audio'),
}
GENERIC = {'video': ('video', '#0e7490'), 'audio': ('audio', '#7c3aed')}

LIGHT = {'paper': (255, 255, 255), 'outline': (176, 182, 194), 'fold': (226, 230, 237), 'lines': (229, 232, 238), 'shadow': 60}
DARK = {'paper': (43, 45, 50), 'outline': (88, 92, 102), 'fold': (62, 65, 72), 'lines': (60, 63, 70), 'shadow': 110}


def hexrgb(h):
    h = h.lstrip('#')
    return tuple(int(h[i:i + 2], 16) for i in (0, 2, 4))


def page(size, label, color, detail, dark=False):
    """Ein Symbol in 'size' Pixeln, gezeichnet in 4-facher Größe und verkleinert (glatte Kanten)."""
    P = DARK if dark else LIGHT
    col = hexrgb(color)
    if dark:  # auf dunklem Blatt aufhellen, damit die Endung lesbar bleibt
        col = tuple(round(c + (255 - c) * 0.38) for c in col)
    S = size * 4
    u = S / 256
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    left, top, right, bottom, fold = 40 * u, 10 * u, 216 * u, 246 * u, 52 * u
    if size <= 24:
        left, right, top, bottom, fold = 34 * u, 222 * u, 6 * u, 250 * u, 60 * u
    shadow = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    ImageDraw.Draw(shadow).polygon([(left, top + 4 * u), (right - fold, top + 4 * u), (right, top + fold + 4 * u), (right, bottom + 4 * u), (left, bottom + 4 * u)], fill=(0, 0, 0, P['shadow']))
    img.alpha_composite(shadow.filter(ImageFilter.GaussianBlur(5 * u)))
    outline = P['outline'] + (255,)
    w = max(1, round(3 * u)) if size >= 32 else max(1, round(5 * u))
    d.polygon([(left, top), (right - fold, top), (right, top + fold), (right, bottom), (left, bottom)], fill=P['paper'] + (255,), outline=outline, width=w)
    d.polygon([(right - fold, top), (right - fold, top + fold), (right, top + fold)], fill=P['fold'] + (255,), outline=outline, width=w)
    if detail:
        y = top + 34 * u
        while y < bottom - 34 * u:
            x2 = right - (fold + 14 * u if y < top + fold + 6 * u else 22 * u)
            d.rounded_rectangle([left + 20 * u, y, x2, y + 7 * u], radius=3.5 * u, fill=P['lines'] + (255,))
            y += 19 * u
    ls = (104 if detail else 150) * u
    cx = (left + right) / 2
    cy = (top + 98 * u) if detail else (top + bottom) / 2
    if detail:
        d.rounded_rectangle([cx - ls / 2 - 7 * u, cy - ls / 2 - 7 * u, cx + ls / 2 + 7 * u, cy + ls / 2 + 7 * u], radius=30 * u, fill=P['paper'] + (255,))
    img.alpha_composite(LOGO.resize((round(ls), round(ls)), Image.LANCZOS), (round(cx - ls / 2), round(cy - ls / 2)))
    if detail and label:
        fs = 58 * u if len(label) <= 3 else (48 * u if len(label) == 4 else 40 * u)
        font = ImageFont.truetype(FONT, round(fs))
        tw = d.textlength(label, font=font)
        ty = bottom - 84 * u
        x0, y0, x1, y1 = d.textbbox((cx - tw / 2, ty - 8 * u), label, font=font)
        d.rectangle([x0 - 8 * u, y0 - 6 * u, x1 + 8 * u, y1 + 6 * u], fill=P['paper'] + (255,))
        d.text((cx - tw / 2, ty - 8 * u), label, font=font, fill=col + (255,))
    if label and size >= 32:  # farbiger Streifen ganz unten
        d.rectangle([left + w, bottom - 16 * u, right - w + 1, bottom], fill=col + (255,))
    return img.resize((size, size), Image.LANCZOS)


def png(img):
    b = io.BytesIO()
    img.save(b, 'PNG', optimize=True)
    return b.getvalue()


def write_ico(path, images):
    head = struct.pack('<HHH', 0, 1, len(images))
    entries, data = b'', b''
    offset = 6 + 16 * len(images)
    for size, blob in images:
        entries += struct.pack('<BBBBHHII', 0 if size >= 256 else size, 0 if size >= 256 else size, 0, 0, 1, 32, len(blob), offset)
        data += blob
        offset += len(blob)
    open(path, 'wb').write(head + entries + data)


icons = {ext: (label, color) for ext, (label, color, _) in TYPES.items()}
icons.update(GENERIC)
for variant in ('light', 'dark'):
    os.makedirs(os.path.join(OUT, variant), exist_ok=True)
    for name, (label, color) in icons.items():
        write_ico(os.path.join(OUT, variant, f'{name}.ico'),
                  [(s, png(page(s, label, color, s >= 48, variant == 'dark'))) for s in (16, 24, 32, 48, 64, 128, 256)])
print(f'{len(icons)} Dateisymbole (hell + dunkel) nach {OUT}')

# ---------------------------------------------------------------- Installer: Registry-Einträge
L = ['; Erzeugt von tools_make_filetype_icons.py - nicht von Hand ändern. Wird in cutline.iss eingebunden.',
     '; Je Dateityp eine ProgID (Cutline.mp3 ...) mit eigenem Dateisymbol. Unter "Öffnen mit" steht Cutline nur über diese',
     '; ProgIDs - ein zusätzlicher Eintrag über Applications\\Cutline.exe würde für alle Typen das Programmsymbol zeigen.']
for ext, (label, color, kind) in TYPES.items():
    L += [
        f'Root: HKA; Subkey: "Software\\Classes\\Cutline.{ext}"; ValueType: string; ValueData: "{label.upper()} ({kind})"; Flags: uninsdeletekey; Tasks: openwith',
        f'Root: HKA; Subkey: "Software\\Classes\\Cutline.{ext}\\DefaultIcon"; ValueType: string; ValueData: "{{app}}\\filetypes\\light\\{ext}.ico"; Tasks: openwith',
        f'Root: HKA; Subkey: "Software\\Classes\\Cutline.{ext}\\shell\\open\\command"; ValueType: string; ValueData: """{{app}}\\Cutline.exe"" ""%1"""; Tasks: openwith',
        f'Root: HKA; Subkey: "Software\\Classes\\.{ext}\\OpenWithProgids"; ValueType: string; ValueName: "Cutline.{ext}"; ValueData: ""; Flags: uninsdeletevalue; Tasks: openwith',
        f'Root: HKA; Subkey: "Software\\Cutline\\Capabilities\\FileAssociations"; ValueType: string; ValueName: ".{ext}"; ValueData: "Cutline.{ext}"; Tasks: openwith',
        # aufräumen: Einträge älterer Versionen (gemeinsame ProgID, Applications-Liste)
        f'Root: HKA; Subkey: "Software\\Classes\\.{ext}\\OpenWithList\\Cutline.exe"; ValueType: none; Flags: deletekey',
        f'Root: HKA; Subkey: "Software\\Classes\\.{ext}\\OpenWithProgids"; ValueType: none; ValueName: "Cutline.{"Video" if kind == "video" else "Audio"}"; Flags: deletevalue',
        # Cutline kann das Symbol (hell/dunkel) für den Benutzer umgestellt haben: beim Deinstallieren mit entfernen
        f'Root: HKCU; Subkey: "Software\\Classes\\Cutline.{ext}"; Flags: uninsdeletekey dontcreatekey',
    ]
L += [
    'Root: HKA; Subkey: "Software\\Classes\\Applications\\Cutline.exe\\SupportedTypes"; ValueType: none; Flags: deletekey',
    '; Sammel-ProgIDs älterer Versionen bleiben (falls Windows sie als Standard gespeichert hat), mit neuem Symbol',
]
for kind in ('Video', 'Audio'):
    L += [
        f'Root: HKA; Subkey: "Software\\Classes\\Cutline.{kind}"; ValueType: string; ValueData: "{kind}"; Flags: uninsdeletekey; Tasks: openwith',
        f'Root: HKA; Subkey: "Software\\Classes\\Cutline.{kind}\\DefaultIcon"; ValueType: string; ValueData: "{{app}}\\filetypes\\light\\{kind.lower()}.ico"; Tasks: openwith',
        f'Root: HKA; Subkey: "Software\\Classes\\Cutline.{kind}\\shell\\open\\command"; ValueType: string; ValueData: """{{app}}\\Cutline.exe"" ""%1"""; Tasks: openwith',
        f'Root: HKCU; Subkey: "Software\\Classes\\Cutline.{kind}"; Flags: uninsdeletekey dontcreatekey',
    ]
open(os.path.join(ROOT, 'packaging', 'windows', 'filetypes.iss'), 'w', encoding='utf-8-sig', newline='\r\n').write('\n'.join(L) + '\n')
print('packaging/windows/filetypes.iss geschrieben')
