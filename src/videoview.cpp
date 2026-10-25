#include "videoview.h"

#include <QGraphicsItem>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include "theme.h"

VideoView::VideoView(QGraphicsScene* scene, QWidget* parent) : QGraphicsView(scene, parent) {
    setRenderHint(QPainter::Antialiasing);
    // Videobild beim Verkleinern/Vergrößern glätten. Ohne das skaliert der Software-Pfad (Linux, kein GPU-Umweg)
    // per Nearest-Neighbor: Kanten werden treppig, kleine Schrift im Video unlesbar - "schlechte Auflösung".
    setRenderHint(QPainter::SmoothPixmapTransform);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setFrameShape(QFrame::NoFrame);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    // Hineingezogene Dateien nicht selbst schlucken: das Hauptfenster entscheidet, was damit passiert
    setAcceptDrops(false);
    viewport()->setAcceptDrops(false);
}

void VideoView::setAmbient(const QImage& tiny) {
    ambient_ = tiny;
    viewport()->update();
}

void VideoView::setToolActive(bool on) {
    tool_ = on;
    toolDown_ = false;
    if (on) viewport()->setCursor(Qt::CrossCursor);
    else viewport()->unsetCursor();
}

void VideoView::setAudioTitle(const QString& title) {
    audioTitle_ = title;
    viewport()->update();
}

// Tondatei: großes Noten-Symbol und der Dateiname mitten im Bild
void VideoView::drawForeground(QPainter* p, const QRectF&) {
    if (audioTitle_.isEmpty()) return;
    p->save();
    p->resetTransform();
    p->setRenderHint(QPainter::Antialiasing);
    const QRectF vr = viewport()->rect();
    const double s = std::min(vr.width(), vr.height()) * 0.22;
    const QPointF c(vr.center().x(), vr.center().y() - s * 0.25);
    QLinearGradient g(c - QPointF(s, s), c + QPointF(s, s));
    g.setColorAt(0, currentTheme().accent);
    g.setColorAt(1, currentTheme().accent.darker(170));
    p->setPen(Qt::NoPen);
    p->setBrush(g);
    p->drawRoundedRect(QRectF(c.x() - s, c.y() - s, 2 * s, 2 * s), s * 0.22, s * 0.22);
    // Note
    p->setBrush(Qt::white);
    const double k = s / 50.0;
    p->drawEllipse(QPointF(c.x() - 14 * k, c.y() + 18 * k), 10 * k, 8 * k);
    p->drawEllipse(QPointF(c.x() + 18 * k, c.y() + 12 * k), 10 * k, 8 * k);
    QPen pen(Qt::white, 5 * k, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen(pen);
    p->drawLine(QPointF(c.x() - 5 * k, c.y() + 16 * k), QPointF(c.x() - 5 * k, c.y() - 22 * k));
    p->drawLine(QPointF(c.x() + 27 * k, c.y() + 10 * k), QPointF(c.x() + 27 * k, c.y() - 28 * k));
    p->drawLine(QPointF(c.x() - 5 * k, c.y() - 22 * k), QPointF(c.x() + 27 * k, c.y() - 28 * k));
    QFont f = font();
    f.setPixelSize(std::max(13, int(s * 0.16)));
    f.setBold(true);
    p->setFont(f);
    p->setPen(QColor(255, 255, 255, 230));
    const QRectF tr(vr.left() + 20, c.y() + s + 14, vr.width() - 40, s * 0.4);
    p->drawText(tr, Qt::AlignHCenter | Qt::AlignTop,
                QFontMetrics(f).elidedText(audioTitle_, Qt::ElideMiddle, int(tr.width())));
    p->restore();
}

// Windows XP: sanfte Hügellandschaft mit Himmel ("Bliss"-Stimmung), wenn noch kein Video geladen ist
void VideoView::paintBliss(QPainter* p, const QRect& r) {
    p->setRenderHint(QPainter::Antialiasing, true);
    QLinearGradient sky(0, r.top(), 0, r.bottom());
    sky.setColorAt(0.0, QColor("#1f5fcf"));
    sky.setColorAt(0.45, QColor("#5b9bee"));
    sky.setColorAt(0.75, QColor("#a9cff7"));
    sky.setColorAt(1.0, QColor("#d8ecff"));
    p->fillRect(r, sky);
    // Wolken
    auto cloud = [&](double cx, double cy, double w, double h, int a) {
        QRadialGradient g(QPointF(r.left() + cx * r.width(), r.top() + cy * r.height()), w * r.width());
        g.setColorAt(0, QColor(255, 255, 255, a));
        g.setColorAt(1, QColor(255, 255, 255, 0));
        p->save();
        p->translate(r.left() + cx * r.width(), r.top() + cy * r.height());
        p->scale(1.0, h / w);
        p->translate(-(r.left() + cx * r.width()), -(r.top() + cy * r.height()));
        p->fillRect(r.adjusted(-r.width(), -r.height(), r.width(), r.height()), g);
        p->restore();
    };
    cloud(0.22, 0.28, 0.22, 0.07, 170);
    cloud(0.62, 0.18, 0.28, 0.06, 150);
    cloud(0.86, 0.36, 0.16, 0.05, 130);
    // Hügel
    const double W = r.width(), H = r.height();
    QPainterPath back;
    back.moveTo(r.left(), r.top() + H * 0.78);
    back.cubicTo(r.left() + W * 0.25, r.top() + H * 0.62, r.left() + W * 0.55, r.top() + H * 0.70, r.left() + W, r.top() + H * 0.66);
    back.lineTo(r.right() + 1, r.bottom() + 1);
    back.lineTo(r.left(), r.bottom() + 1);
    back.closeSubpath();
    QLinearGradient gb(0, r.top() + H * 0.62, 0, r.bottom());
    gb.setColorAt(0, QColor("#7cc242"));
    gb.setColorAt(1, QColor("#3f8a1f"));
    p->fillPath(back, gb);
    QPainterPath front;
    front.moveTo(r.left(), r.top() + H * 0.70);
    front.cubicTo(r.left() + W * 0.30, r.top() + H * 0.50, r.left() + W * 0.66, r.top() + H * 0.58, r.left() + W, r.top() + H * 0.92);
    front.lineTo(r.right() + 1, r.bottom() + 1);
    front.lineTo(r.left(), r.bottom() + 1);
    front.closeSubpath();
    QLinearGradient gf(r.left() + W * 0.2, r.top() + H * 0.52, r.left() + W * 0.5, r.bottom());
    gf.setColorAt(0, QColor("#a3dc5a"));
    gf.setColorAt(0.35, QColor("#5fae2e"));
    gf.setColorAt(1, QColor("#2d6f14"));
    p->fillPath(front, gf);
}

void VideoView::drawBackground(QPainter* p, const QRectF&) {
    p->save();
    p->resetTransform();
    const QRect vr = viewport()->rect();
    p->fillRect(vr, currentTheme().video);
    if (currentTheme().xp && ambient_.isNull()) paintBliss(p, vr);
    if (!ambient_.isNull()) {
        p->setRenderHint(QPainter::SmoothPixmapTransform, true);
        p->setOpacity(0.5);
        p->drawImage(vr, ambient_);
        p->setOpacity(1.0);
        p->fillRect(vr, QColor(0, 0, 0, 70));
    }
    p->restore();
}

void VideoView::mousePressEvent(QMouseEvent* e) {
    if (tool_) {
        if (e->button() == Qt::LeftButton) {
            toolDown_ = true;
            emit toolPressed(mapToScene(e->pos()));
        }
        return;
    }
    QGraphicsItem* it = itemAt(e->pos());
    pressOnEmpty_ = e->button() == Qt::LeftButton && (!it || it->zValue() < 5 || it->acceptedMouseButtons() == Qt::NoButton);
    pressPos_ = e->pos();
    QGraphicsView::mousePressEvent(e);
}

void VideoView::mouseReleaseEvent(QMouseEvent* e) {
    if (tool_) {
        if (toolDown_ && e->button() == Qt::LeftButton) {
            toolDown_ = false;
            emit toolReleased(mapToScene(e->pos()));
        }
        return;
    }
    QGraphicsView::mouseReleaseEvent(e);
    if (pressOnEmpty_ && (e->pos() - pressPos_).manhattanLength() < 6) emit emptyClicked();
    pressOnEmpty_ = false;
}

void VideoView::mouseMoveEvent(QMouseEvent* e) {
    if (tool_) {
        if (toolDown_) emit toolMoved(mapToScene(e->pos()));
        emit mouseActivity();
        return;
    }
    QGraphicsView::mouseMoveEvent(e);
    emit mouseActivity();
}
