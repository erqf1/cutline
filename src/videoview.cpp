#include "videoview.h"

#include <QGraphicsItem>
#include <QMouseEvent>
#include <QPainter>
#include "theme.h"

VideoView::VideoView(QGraphicsScene* scene, QWidget* parent) : QGraphicsView(scene, parent) {
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setFrameShape(QFrame::NoFrame);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
}

void VideoView::setAmbient(const QImage& tiny) {
    ambient_ = tiny;
    viewport()->update();
}

void VideoView::drawBackground(QPainter* p, const QRectF&) {
    p->save();
    p->resetTransform();
    const QRect vr = viewport()->rect();
    p->fillRect(vr, currentTheme().video);
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
    QGraphicsItem* it = itemAt(e->pos());
    pressOnEmpty_ = e->button() == Qt::LeftButton && (!it || it->zValue() < 5 || it->acceptedMouseButtons() == Qt::NoButton);
    pressPos_ = e->pos();
    QGraphicsView::mousePressEvent(e);
}

void VideoView::mouseReleaseEvent(QMouseEvent* e) {
    QGraphicsView::mouseReleaseEvent(e);
    if (pressOnEmpty_ && (e->pos() - pressPos_).manhattanLength() < 6) emit emptyClicked();
    pressOnEmpty_ = false;
}

void VideoView::mouseMoveEvent(QMouseEvent* e) {
    QGraphicsView::mouseMoveEvent(e);
    emit mouseActivity();
}
