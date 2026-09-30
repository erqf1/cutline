#pragma once
#include <QGraphicsItem>
#include <QPixmap>
#include "host.h"

// Verschieb- und skalierbares Element (Bild oder Unschärfe-Bereich) auf dem Videobild.
class Overlay : public QGraphicsItem {
public:
    Overlay(int itemId, EditorHost* host);
    int itemId() const { return id_; }
    QRectF boundingRect() const override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

private:
    QRectF handleRect() const;
    void paintBlur(QPainter* p);
    void syncModel();

    int id_;
    EditorHost* host_;
    QPixmap pm_;
    QImage textImg_;
    QString textKey_;
    double w_ = 0, h_ = 0;
    enum Mode { None, Move, Resize } mode_ = None;
    bool moved_ = false;
    QPointF startScene_, startPos_;
    double startW_ = 0, startH_ = 0;
};
