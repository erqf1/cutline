#pragma once
#include <QWidget>
#include "host.h"

class Timeline : public QWidget {
public:
    explicit Timeline(EditorHost* host);
    void relayout();

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    static constexpr int RULER = 26, ROW = 72, LANE = 24, LEFT = 14;
    double pps() const;
    double xOf(double t) const { return LEFT + t * pps(); }
    double tOf(double x) const { return std::max(0.0, (x - LEFT) / pps()); }
    int laneY(int lane) const { return RULER + ROW + 6 + lane * LANE; }
    Item* itemAt(const QPointF& pos);
    AudioClip* audioAt(const QPointF& pos);

    struct Drag {
        enum Kind { None, Seek, Trim, ItemDrag, AudioDrag } kind = None;
        int index = 0;      // Abschnitt-Index bzw. ID
        char side = 0;      // 'l', 'r', 'm'
        double x0 = 0, a = 0, b = 0, c = 0;
    } drag_;

    EditorHost* host_;
    double zoom_ = 1.0;
};
