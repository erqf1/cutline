#pragma once
#include <QList>
#include <QSize>
#include <QString>
#include <QStringList>
#include <algorithm>
#include <cmath>
#include <cstdlib>

struct Source {
    QString path;
    double duration = 0;
    double fps = 0;
    bool hasAudio = false;
    bool probed = false;
};

struct Piece {
    int src = 0;
    double start = 0, end = 0, speed = 1.0;
    double volume = 1.0;  // Lautstärke des Video-Tons in diesem Abschnitt (0..6 = bis 600 %)
    bool denoise = false;  // Rauschunterdrückung (Klicks, Rauschen) beim Export
    // Bild anpassen: Größe (1 = eingepasst), Verschiebung als Anteil der Bildfläche, Drehung in Grad
    double scale = 1.0, px = 0, py = 0, rot = 0;
    double outDur() const { return (end - start) / speed; }
    bool transformed() const {
        return std::abs(scale - 1) > 1e-4 || std::abs(px) > 1e-4 || std::abs(py) > 1e-4 || std::abs(rot) > 1e-3;
    }
};

struct Item {
    enum Kind { Image, Blur, Text };
    int id = 0;
    Kind kind = Blur;
    double t0 = 0, t1 = 0;              // Ausgabezeit
    double x = 0, y = 0, w = 0, h = 0;  // normalisiert (0..1) relativ zum Bild
    QString path;
    double strength = 30.0;             // Blur-Sigma (bei 1080p)
    double ar = 1.0;                    // Seitenverhältnis h/w (Bild)
    // Text: Inhalt, Schriftart, Farbe (ARGB), optional Hintergrund
    QString text, font;
    unsigned color = 0xffffffffu, bgColor = 0xcc000000u;
    bool bg = false;
};

struct AudioClip {
    int id = 0;
    QString path;
    double t0 = 0;        // Startzeit in der Ausgabe
    double srcStart = 0;  // Startpunkt in der Datei
    double dur = 0;       // Länge in der Ausgabe
    double fileDur = 0;
    double volume = 1.0;
    bool denoise = false;  // Rauschunterdrückung beim Export
};

struct Snapshot {
    QList<Piece> pieces;
    QList<Item> items;
    QList<AudioClip> audios;
    int aspect = 0;
};

// Bildformate der Ausgabe; w == 0 heißt "wie das erste Video"
struct AspectFormat { int w, h; const char* label; };
constexpr int kAspectCount = 7;
extern const AspectFormat kAspects[kAspectCount];
// Größe der Bildfläche: die kurze Seite des Originals bleibt (z. B. 1920×1080 -> 9:16 = 1080×1920)
QSize canvasSize(int aspect, int natW, int natH);

class Project {
public:
    QList<Source> sources;
    QList<Piece> pieces;
    QList<Item> items;
    QList<AudioClip> audios;
    int vw = 1920, vh = 1080;  // Bildfläche (aus Bildformat und Größe des ersten Videos)
    int natW = 0, natH = 0;    // Größe des ersten Videos
    int aspect = 0;            // Index in kAspects
    int nextId = 1;

    double total() const {
        double t = 0;
        for (const Piece& p : pieces) t += p.outDur();
        return t;
    }
    double outStart(int i) const {
        double t = 0;
        for (int k = 0; k < i && k < pieces.size(); ++k) t += pieces[k].outDur();
        return t;
    }
    // Ausgabezeit -> Abschnitt-Index + Quellzeit
    int locate(double t, double* src) const {
        double acc = 0;
        for (int i = 0; i < pieces.size(); ++i) {
            const Piece& p = pieces[i];
            if (t < acc + p.outDur() || i == pieces.size() - 1) {
                if (src) *src = p.start + std::clamp(t - acc, 0.0, p.outDur()) * p.speed;
                return i;
            }
            acc += p.outDur();
        }
        if (src) *src = 0;
        return 0;
    }
    double srcDuration(int i) const { return sources.value(pieces.value(i).src).duration; }
    Item* item(int id) {
        for (Item& it : items) if (it.id == id) return &it;
        return nullptr;
    }
    AudioClip* audio(int id) {
        for (AudioClip& a : audios) if (a.id == id) return &a;
        return nullptr;
    }
    Snapshot snapshot() const { return {pieces, items, audios, aspect}; }
    void restore(const Snapshot& s) { pieces = s.pieces; items = s.items; audios = s.audios; aspect = s.aspect; }
};

QString fmtTime(double t);

struct MediaInfo {
    bool ok = false;
    double duration = 0, fps = 0;
    bool hasAudio = false;
    int w = 0, h = 0;
};

// ---- ffmpeg
QString ffmpegPath();
// RNNoise-Modell für ffmpegs arnndn (aus den Ressourcen in den Cache entpackt)
QString denoiseModelPath();
QString detectEncoder();
MediaInfo probeMedia(const QString& path);

struct ExportOptions {
    int width = 1920, height = 1080;
    double fps = 30;
    int quality = 1;  // 0 hoch, 1 ausgewogen, 2 klein
    QString encoder = "libx264";
    QString format = "mp4";  // mp4 mov mkv gif | mp3 wav m4a (nur Ton)
};

bool isAudioFormat(const QString& format);

QStringList buildExport(const Project& pr, const ExportOptions& o, const QString& out);
