#pragma once
#include <QGraphicsView>
#include <QImage>

// Videoansicht: weiches "Ambient"-Licht in den freien Rändern, Klick auf leeres Video = Pause/Play.
class VideoView : public QGraphicsView {
    Q_OBJECT
public:
    explicit VideoView(QGraphicsScene* scene, QWidget* parent = nullptr);
    void setAmbient(const QImage& tiny);

signals:
    void emptyClicked();
    void mouseActivity();

protected:
    void drawBackground(QPainter* p, const QRectF& r) override;
    void paintBliss(QPainter* p, const QRect& r);
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

private:
    QImage ambient_;
    bool pressOnEmpty_ = false;
    QPoint pressPos_;
};
