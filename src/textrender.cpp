#include "textrender.h"

#include <QDirIterator>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QTextLayout>

static QStringList g_special;

void registerSpecialFonts() {
    if (!g_special.isEmpty()) return;
    QDirIterator it(":/fonts", {"*.ttf", "*.otf"});
    QStringList files;
    while (it.hasNext()) files << it.next();
    files.sort();
    for (const QString& f : files) {
        const int id = QFontDatabase::addApplicationFont(f);
        if (id >= 0) g_special << QFontDatabase::applicationFontFamilies(id);
    }
    g_special.removeDuplicates();
}

QStringList specialFonts() { return g_special; }

QString defaultTextFont() { return QGuiApplication::font().family(); }

// Größte Schrift, bei der alle Zeilen in den Rahmen passen
static QFont fitFont(const QString& family, const QStringList& lines, QSizeF box) {
    QFont f(family.isEmpty() ? defaultTextFont() : family);
    double lo = 4, hi = std::max(8.0, box.height() * 1.2);
    for (int k = 0; k < 18; ++k) {
        const double mid = (lo + hi) / 2;
        f.setPixelSize(std::max(1, int(mid)));
        const QFontMetricsF fm(f);
        double w = 0;
        for (const QString& l : lines) w = std::max(w, fm.horizontalAdvance(l));
        const double h = fm.height() * lines.size();
        (w <= box.width() && h <= box.height()) ? lo = mid : hi = mid;
    }
    f.setPixelSize(std::max(1, int(lo)));
    return f;
}

QImage renderTextImage(const Item& it, QSize size) {
    size = size.expandedTo(QSize(2, 2));
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    const QRectF r(0, 0, size.width(), size.height());
    const double pad = it.bg ? std::min(r.width(), r.height()) * 0.12 : std::min(r.width(), r.height()) * 0.04;
    if (it.bg) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor::fromRgba(it.bgColor));
        const double rad = std::min(r.width(), r.height()) * 0.18;
        p.drawRoundedRect(r, rad, rad);
    }
    const QString text = it.text.isEmpty() ? QString(" ") : it.text;
    const QStringList lines = text.split('\n');
    const QRectF box = r.adjusted(pad, pad, -pad, -pad);
    const QFont f = fitFont(it.font, lines, box.size());
    const QFontMetricsF fm(f);
    const double lineH = fm.height();
    double y = box.top() + (box.height() - lineH * lines.size()) / 2 + fm.ascent();
    QPainterPath path;
    for (const QString& l : lines) {
        const double x = box.left() + (box.width() - fm.horizontalAdvance(l)) / 2;
        path.addText(QPointF(x, y), f, l);
        y += lineH;
    }
    // ohne Hintergrund: dunkle Kontur, damit der Text auf jedem Bild lesbar bleibt
    if (!it.bg) {
        QPen outline(QColor(0, 0, 0, 170), std::max(1.0, f.pixelSize() * 0.09), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.strokePath(path, outline);
    }
    p.fillPath(path, QColor::fromRgba(it.color));
    return img;
}
