#pragma once
#include <QColor>
#include <QList>
#include <QString>

struct Theme {
    QString id, name;
    QColor bg, panel, panel2, border, text, muted, accent, accentText, timeline, video;
    QString font;
    int fontPx = 13;
    int radius = 8;
    bool xp = false;
};

const QList<Theme>& themes();
const Theme& currentTheme();
void setCurrentTheme(const QString& id);
QString buildStyleSheet(const Theme& t);

// Feste Farben für Timeline-Elemente (in jedem Theme gut lesbar)
inline QColor blurColor() { return QColor("#e8912d"); }
inline QColor imageColor() { return QColor("#4a90e2"); }
inline QColor textItemColor() { return QColor("#d9488f"); }
inline QColor audioColor() { return QColor("#9b6be0"); }
inline QColor playheadColor() { return QColor("#ff5d5d"); }
