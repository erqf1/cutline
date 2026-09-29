#pragma once
#include <QList>
#include <QString>
#include <QStringList>
#include <algorithm>

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
    double outDur() const { return (end - start) / speed; }
};

struct Item {
    enum Kind { Image, Blur };
    int id = 0;
    Kind kind = Blur;
    double t0 = 0, t1 = 0;              // Ausgabezeit
    double x = 0, y = 0, w = 0, h = 0;  // normalisiert (0..1) relativ zum Bild
    QString path;
    double strength = 30.0;             // Blur-Sigma (bei 1080p)
    double ar = 1.0;                    // Seitenverhältnis h/w (Bild)
};

struct AudioClip {
    int id = 0;
    QString path;
    double t0 = 0;        // Startzeit in der Ausgabe
    double srcStart = 0;  // Startpunkt in der Datei
    double dur = 0;       // Länge in der Ausgabe
    double fileDur = 0;
    double volume = 1.0;
};

struct Snapshot {
    QList<Piece> pieces;
    QList<Item> items;
    QList<AudioClip> audios;
};

class Project {
public:
    QList<Source> sources;
    QList<Piece> pieces;
    QList<Item> items;
    QList<AudioClip> audios;
    int vw = 1920, vh = 1080;  // Bildfläche (Größe des ersten Videos)
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
    Snapshot snapshot() const { return {pieces, items, audios}; }
    void restore(const Snapshot& s) { pieces = s.pieces; items = s.items; audios = s.audios; }
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
QString detectEncoder();
MediaInfo probeMedia(const QString& path);

struct ExportOptions {
    int width = 1920, height = 1080;
    double fps = 30;
    int quality = 1;  // 0 hoch, 1 ausgewogen, 2 klein
    QString encoder = "libx264";
};

QStringList buildExport(const Project& pr, const ExportOptions& o, const QString& out);
