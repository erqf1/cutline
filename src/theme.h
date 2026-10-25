#pragma once
#include <QColor>
#include <QList>
#include <QString>

struct Theme {
    QString id, name;
    QColor bg, panel, panel2, border, text, muted, accent, accentText, timeline, video;
    QColor hover;      // Hintergrund beim Überfahren (YouTube-Stil)
    QString font;
    int fontPx = 13;
    int radius = 8;
    bool xp = false;
    bool yt = false;   // runde "Chip"-Knöpfe ohne Rahmen wie bei YouTube; ausgewählt = Textfarbe als Fläche
};

const QList<Theme>& themes();
const Theme& currentTheme();
void setCurrentTheme(const QString& id);
// Eigene Akzentfarbe (Knöpfe, Fortschritt, Auswahl); leer = Farbe des Designs. Windows XP behält seine Farben.
void setAccentColor(const QString& color);
QString accentColor();
const QList<QColor>& accentChoices();
QString buildStyleSheet(const Theme& t);

// Feste Farben für Timeline-Elemente (in jedem Theme gut lesbar)
inline QColor blurColor() { return QColor("#e8912d"); }
inline QColor imageColor() { return QColor("#4a90e2"); }
inline QColor textItemColor() { return QColor("#d9488f"); }
inline QColor audioColor() { return QColor("#9b6be0"); }
inline QColor drawItemColor() { return QColor("#2bb673"); }
inline QColor shapeItemColor() { return QColor("#14a3a8"); }
inline QColor playheadColor() { return QColor("#ff5d5d"); }
