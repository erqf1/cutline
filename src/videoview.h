#pragma once
#include <QGraphicsView>
#include <QImage>

// Videoansicht: weiches "Ambient"-Licht in den freien Rändern, Klick auf leeres Video = Pause/Play.
// Mit aktivem Werkzeug (Zeichnen / Form) gehen alle Mausbewegungen an das Werkzeug statt an die Elemente.
class VideoView : public QGraphicsView {
    Q_OBJECT
public:
    explicit VideoView(QGraphicsScene* scene, QWidget* parent = nullptr);
    void setAmbient(const QImage& tiny);
    void setToolActive(bool on);
    // Reine Tondatei: Name und Symbol statt Bild
    void setAudioTitle(const QString& title);

signals:
    void emptyClicked();
    void mouseActivity();
    void toolPressed(const QPointF& scenePos);
    void toolMoved(const QPointF& scenePos);
    void toolReleased(const QPointF& scenePos);

protected:
    void drawBackground(QPainter* p, const QRectF& r) override;
    void drawForeground(QPainter* p, const QRectF& r) override;
    void paintBliss(QPainter* p, const QRect& r);
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

private:
    QImage ambient_;
    QString audioTitle_;
    bool pressOnEmpty_ = false, tool_ = false, toolDown_ = false;
    QPoint pressPos_;
};
