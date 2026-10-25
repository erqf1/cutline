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

QImage renderGraphicImage(const Item& it, QSize size, double canvasH) {
    size = size.expandedTo(QSize(2, 2));
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    const QColor col = QColor::fromRgba(it.color);
    const double sw = std::max(1.0, it.stroke * canvasH / 1080.0);
    QPen pen(col, sw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    const double W = size.width(), H = size.height();
    auto map = [W, H](const QPointF& q) { return QPointF(q.x() * W, q.y() * H); };
    if (it.kind == Item::Draw) {
        p.setPen(pen);
        for (const QPolygonF& s : it.strokes) {
            if (s.size() == 1) {  // einzelner Klick: Punkt
                p.setPen(Qt::NoPen);
                p.setBrush(col);
                p.drawEllipse(map(s[0]), sw / 2, sw / 2);
                p.setPen(pen);
                continue;
            }
            QPolygonF poly;
            for (const QPointF& q : s) poly << map(q);
            p.drawPolyline(poly);
        }
        return img;
    }
    const QRectF box = QRectF(0, 0, W, H).adjusted(sw / 2, sw / 2, -sw / 2, -sw / 2);
    p.setPen(pen);
    p.setBrush(it.fill ? QBrush(col) : QBrush(Qt::NoBrush));
    if (it.shape == Item::Rect) {
        const double r = std::min(box.width(), box.height()) * 0.06;
        p.drawRoundedRect(box, r, r);
    } else if (it.shape == Item::Ellipse) {
        p.drawEllipse(box);
    } else {
        // Linie / Pfeil von strokes[0][0] nach strokes[0][1]
        QPointF a(0, 1), b(1, 0);
        if (!it.strokes.isEmpty() && it.strokes[0].size() >= 2) { a = it.strokes[0][0]; b = it.strokes[0][1]; }
        auto inBox = [&box](const QPointF& q) { return QPointF(box.left() + q.x() * box.width(), box.top() + q.y() * box.height()); };
        QPointF pa = inBox(a), pb = inBox(b);
        if (it.shape == Item::Arrow) {
            const QLineF l(pb, pa);
            const double head = std::min(l.length() * 0.45, sw * 4.5 + 10);
            QLineF w1 = l, w2 = l;
            w1.setLength(head); w2.setLength(head);
            w1.setAngle(l.angle() + 28); w2.setAngle(l.angle() - 28);
            QPolygonF tip({pb, w1.p2(), w2.p2()});
            p.drawLine(pa, pb);
            p.setBrush(col);
            p.drawPolygon(tip);
        } else {
            p.drawLine(pa, pb);
        }
    }
    return img;
}

QImage renderItemImage(const Item& it, QSize size, double canvasH) {
    return it.kind == Item::Text ? renderTextImage(it, size) : renderGraphicImage(it, size, canvasH);
}
