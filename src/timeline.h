#pragma once
#include <QWidget>
#include "host.h"

class Timeline : public QWidget {
public:
    explicit Timeline(EditorHost* host);
    void relayout();
    void zoomBy(double factor);
    double playheadX() const { return xOf(host_->curTime()); }

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    static constexpr int RULER = 26, ROW = 86, WAVE = 26, LANE = 32, LEFT = 14;
    double basePps() const;
    double pps() const;
    double xOf(double t) const { return LEFT + t * pps(); }
    double tOf(double x) const { return std::max(0.0, (x - LEFT) / pps()); }
    int laneY(int lane) const { return RULER + ROW + 6 + lane * LANE; }
    QRectF pieceRect(int i) const;
    QRectF waveRect(int i) const;  // Ton-Streifen unter den Vorschaubildern
    QRectF audioRect(int k) const;
    double volLineY(const QRectF& area, double volume) const;
    Item* itemAt(const QPointF& pos);
    AudioClip* audioAt(const QPointF& pos, int* lane = nullptr);
    int pieceEdgeAt(const QPointF& pos, char* side) const;
    int pieceVolAt(const QPointF& pos) const;
    AudioClip* audioVolAt(const QPointF& pos);
    void drawWave(QPainter& p, const QRectF& area, const WaveSet* ws, double srcAtLeft, double srcPerPx,
                  double volume, bool centered, const QColor& color, const QRect& vis) const;
    void drawDbPill(QPainter& p, double x, double y, double volume) const;

    struct Drag {
        enum Kind { None, Seek, Trim, ItemDrag, AudioDrag, PieceVol, AudioVol } kind = None;
        int index = 0;      // Abschnitt-Index bzw. ID
        char side = 0;      // 'l', 'r', 'm'
        double x0 = 0, a = 0, b = 0, c = 0, y0 = 0;
    } drag_;
    // Lautstärke-Linie unter der Maus (für die dB-Anzeige)
    enum class Hover { None, Piece, Audio } hover_ = Hover::None;
    int hoverIndex_ = -1;
    double hoverX_ = 0;

    EditorHost* host_;
    double zoom_ = 1.0;
};
