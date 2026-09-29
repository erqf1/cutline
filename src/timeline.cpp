#include "timeline.h"

#include <QFileInfo>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include "i18n.h"
#include "theme.h"

static constexpr double kMinPiece = 0.1;

Timeline::Timeline(EditorHost* host) : host_(host) {
    setMouseTracking(true);
    setMinimumHeight(RULER + ROW + LANE * 2 + 16);
}

double Timeline::pps() const {
    double vw = host_->timelineViewportWidth() - 2 * LEFT;
    return std::max(1.0, vw / std::max(host_->pr().total(), 0.5)) * zoom_;
}

void Timeline::relayout() {
    Project& pr = host_->pr();
    int lanes = std::max<int>(2, pr.items.size() + pr.audios.size());
    setMinimumHeight(RULER + ROW + LANE * lanes + 16);
    int w = int(pr.total() * pps()) + 2 * LEFT;
    setFixedWidth(std::max(w, host_->timelineViewportWidth()));
    update();
}

static QColor mix(const QColor& a, const QColor& b, double t) {
    return QColor::fromRgbF(a.redF() * (1 - t) + b.redF() * t, a.greenF() * (1 - t) + b.greenF() * t,
                            a.blueF() * (1 - t) + b.blueF() * t);
}

void Timeline::paintEvent(QPaintEvent* ev) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const Theme& th = currentTheme();
    p.fillRect(rect(), th.timeline);
    Project& pr = host_->pr();
    const double sc = pps(), total = pr.total();
    const QRect vis = ev->rect();

    // Lineal
    double step = 600;
    for (double s : {0.5, 1.0, 2.0, 5.0, 10.0, 15.0, 30.0, 60.0, 120.0, 300.0, 600.0})
        if (s * sc >= 70) { step = s; break; }
    QFont small = font();
    small.setPixelSize(11);
    p.setFont(small);
    for (double t = 0; t <= total + 0.001; t += step) {
        int x = int(xOf(t));
        p.setPen(th.border);
        p.drawLine(x, RULER - 8, x, RULER - 1);
        p.setPen(th.muted);
        p.drawText(x + 4, 14, fmtTime(t));
        for (int m = 1; m < 5; ++m) {
            int xm = int(xOf(t + step * m / 5.0));
            p.setPen(th.border);
            p.drawLine(xm, RULER - 4, xm, RULER - 1);
        }
    }

    // Video-Abschnitte mit Vorschaubildern
    const QColor pieceBase = mix(th.panel2, th.accent, 0.35);
    for (int i = 0; i < pr.pieces.size(); ++i) {
        const Piece& pc = pr.pieces[i];
        double x0 = xOf(pr.outStart(i)), x1 = xOf(pr.outStart(i) + pc.outDur());
        if (x1 < vis.left() - 4 || x0 > vis.right() + 4) continue;
        QRectF r(x0 + 1, RULER + 2, std::max(2.0, x1 - x0 - 2), ROW - 6);
        const bool sel = i == host_->selPiece();
        QPainterPath clip;
        clip.addRoundedRect(r, 7, 7);
        p.save();
        p.setClipPath(clip);
        p.fillRect(r, pieceBase);
        if (const ThumbSet* ts = host_->thumbs(pc.src); ts && !ts->imgs.isEmpty()) {
            const QImage& first = ts->imgs.first();
            const double tw = r.height() * first.width() / std::max(1, first.height());
            const double from = std::max(r.left(), double(vis.left()) - tw);
            const double to = std::min(r.right(), double(vis.right()) + tw);
            for (double tx = r.left() + std::floor((from - r.left()) / tw) * tw; tx < to; tx += tw) {
                double tOut = (tx + tw / 2 - x0) / sc;
                double src = pc.start + std::clamp(tOut, 0.0, pc.outDur()) * pc.speed;
                int idx = std::clamp(int(src / ts->step), 0, int(ts->imgs.size()) - 1);
                p.drawImage(QRectF(tx, r.top(), tw + 1, r.height()), ts->imgs[idx]);
            }
        }
        QLinearGradient shade(0, r.top(), 0, r.bottom());
        shade.setColorAt(0, QColor(0, 0, 0, 20));
        shade.setColorAt(1, QColor(0, 0, 0, 150));
        p.fillRect(r, shade);
        if (sel) p.fillRect(r, QColor(th.accent.red(), th.accent.green(), th.accent.blue(), 45));
        p.restore();
        p.setBrush(Qt::NoBrush);
        p.setPen(sel ? QPen(th.accent, 2.5) : QPen(QColor(0, 0, 0, 90), 1));
        p.drawRoundedRect(r, 7, 7);
        if (sel) {  // Trim-Griffe
            p.setPen(Qt::NoPen);
            p.setBrush(th.accent);
            p.drawRoundedRect(QRectF(r.left() - 1, r.center().y() - 12, 5, 24), 2.5, 2.5);
            p.drawRoundedRect(QRectF(r.right() - 4, r.center().y() - 12, 5, 24), 2.5, 2.5);
        }
        QString label = fmtTime(pc.outDur());
        if (pc.speed != 1.0) label += QString("  ×%1").arg(pc.speed, 0, 'g', 3);
        QFontMetrics fm(small);
        QRectF pill(r.left() + 8, r.bottom() - 21, fm.horizontalAdvance(label) + 12, 16);
        if (pill.right() < r.right()) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, 150));
            p.drawRoundedRect(pill, 8, 8);
            p.setPen(Qt::white);
            p.drawText(pill, Qt::AlignCenter, label);
        }
    }

    // Spuren: erst Elemente (Bild/Unschärfe), dann Ton
    int lane = 0;
    for (const Item& it : pr.items) {
        QRectF r(xOf(it.t0), laneY(lane) + 2, std::max(6.0, (it.t1 - it.t0) * sc), LANE - 5);
        ++lane;
        bool sel = host_->selItem() == it.id;
        QColor base = it.kind == Item::Blur ? blurColor() : imageColor();
        p.setBrush(sel ? base.lighter(125) : base);
        p.setPen(sel ? QPen(th.accent, 2) : QPen(Qt::NoPen));
        p.drawRoundedRect(r, 6, 6);
        p.setPen(Qt::white);
        p.setFont(small);
        p.drawText(r.adjusted(8, 0, -4, 0), Qt::AlignVCenter,
                   it.kind == Item::Blur ? T("blur") : QFileInfo(it.path).fileName());
    }
    for (const AudioClip& a : pr.audios) {
        QRectF r(xOf(a.t0), laneY(lane) + 2, std::max(6.0, a.dur * sc), LANE - 5);
        ++lane;
        bool sel = host_->selAudio() == a.id;
        QColor base = audioColor();
        p.setBrush(sel ? base.lighter(125) : base);
        p.setPen(sel ? QPen(th.accent, 2) : QPen(Qt::NoPen));
        p.drawRoundedRect(r, 6, 6);
        // dezente Wellenform-Andeutung
        p.setPen(QPen(QColor(255, 255, 255, 70), 2, Qt::SolidLine, Qt::RoundCap));
        for (double x = r.left() + 10; x < r.right() - 6; x += 5) {
            double hgt = 3 + 6 * std::abs(std::sin(x * 0.37) * std::cos(x * 0.11));
            p.drawLine(QPointF(x, r.center().y() - hgt), QPointF(x, r.center().y() + hgt));
        }
        p.setPen(Qt::white);
        p.setFont(small);
        p.drawText(r.adjusted(8, 0, -4, 0), Qt::AlignVCenter, QFileInfo(a.path).fileName());
    }

    // Playhead
    int x = int(xOf(host_->curTime()));
    p.setPen(QPen(playheadColor(), 2));
    p.drawLine(x, 0, x, height());
    p.setPen(Qt::NoPen);
    p.setBrush(playheadColor());
    QPainterPath tri;
    tri.moveTo(x - 6, 0); tri.lineTo(x + 6, 0); tri.lineTo(x, 9); tri.closeSubpath();
    p.drawPath(tri);
}

Item* Timeline::itemAt(const QPointF& pos) {
    Project& pr = host_->pr();
    for (int k = 0; k < pr.items.size(); ++k) {
        Item& it = pr.items[k];
        int y = laneY(k);
        if (pos.y() >= y && pos.y() < y + LANE && pos.x() >= xOf(it.t0) - 4 && pos.x() <= xOf(it.t1) + 4)
            return &it;
    }
    return nullptr;
}

AudioClip* Timeline::audioAt(const QPointF& pos) {
    Project& pr = host_->pr();
    for (int k = 0; k < pr.audios.size(); ++k) {
        AudioClip& a = pr.audios[k];
        int y = laneY(pr.items.size() + k);
        if (pos.y() >= y && pos.y() < y + LANE && pos.x() >= xOf(a.t0) - 4 && pos.x() <= xOf(a.t0 + a.dur) + 4)
            return &a;
    }
    return nullptr;
}

void Timeline::mousePressEvent(QMouseEvent* e) {
    const QPointF pos = e->position();
    Project& pr = host_->pr();
    if (pr.pieces.isEmpty()) return;

    if (pos.y() < RULER) {
        drag_.kind = Drag::Seek;
        host_->seek(tOf(pos.x()));
        return;
    }
    if (pos.y() < RULER + ROW) {
        int sel = host_->selPiece();
        if (sel >= 0 && sel < pr.pieces.size()) {
            const Piece& pc = pr.pieces[sel];
            double x0 = xOf(pr.outStart(sel)), x1 = xOf(pr.outStart(sel) + pc.outDur());
            for (char side : {'l', 'r'}) {
                double xe = side == 'l' ? x0 : x1;
                if (std::abs(pos.x() - xe) <= 7) {
                    host_->pushUndo();
                    drag_ = {Drag::Trim, sel, side, pos.x(), pc.start, pc.end, 0};
                    return;
                }
            }
        }
        double t = tOf(pos.x());
        host_->selectPiece(pr.locate(t, nullptr));
        drag_.kind = Drag::Seek;
        host_->seek(t);
        return;
    }
    if (Item* it = itemAt(pos)) {
        host_->selectItem(it->id);
        double x0 = xOf(it->t0), x1 = xOf(it->t1);
        char mode = std::abs(pos.x() - x0) <= 6 ? 'l' : std::abs(pos.x() - x1) <= 6 ? 'r' : 'm';
        host_->pushUndo();
        drag_ = {Drag::ItemDrag, it->id, mode, pos.x(), it->t0, it->t1, 0};
    } else if (AudioClip* a = audioAt(pos)) {
        host_->selectAudio(a->id);
        double x0 = xOf(a->t0), x1 = xOf(a->t0 + a->dur);
        char mode = std::abs(pos.x() - x0) <= 6 ? 'l' : std::abs(pos.x() - x1) <= 6 ? 'r' : 'm';
        host_->pushUndo();
        drag_ = {Drag::AudioDrag, a->id, mode, pos.x(), a->t0, a->dur, a->srcStart};
    } else {
        host_->selectItem(0);
        host_->selectPiece(-1);
    }
}

void Timeline::mouseMoveEvent(QMouseEvent* e) {
    const QPointF pos = e->position();
    Project& pr = host_->pr();
    if (drag_.kind == Drag::None) {
        bool near = false;
        int sel = host_->selPiece();
        if (sel >= 0 && sel < pr.pieces.size() && pos.y() >= RULER && pos.y() < RULER + ROW) {
            double s = pr.outStart(sel);
            near = std::abs(pos.x() - xOf(s)) <= 7 || std::abs(pos.x() - xOf(s + pr.pieces[sel].outDur())) <= 7;
        }
        if (Item* it = itemAt(pos)) near |= std::abs(pos.x() - xOf(it->t0)) <= 6 || std::abs(pos.x() - xOf(it->t1)) <= 6;
        if (AudioClip* a = audioAt(pos)) near |= std::abs(pos.x() - xOf(a->t0)) <= 6 || std::abs(pos.x() - xOf(a->t0 + a->dur)) <= 6;
        setCursor(near ? Qt::SizeHorCursor : Qt::ArrowCursor);
        return;
    }
    if (drag_.kind == Drag::Seek) {
        host_->seek(tOf(pos.x()));
    } else if (drag_.kind == Drag::Trim) {
        if (drag_.index >= pr.pieces.size()) return;
        Piece& pc = pr.pieces[drag_.index];
        double dt = (pos.x() - drag_.x0) / pps() * pc.speed;
        if (drag_.side == 'l') pc.start = std::clamp(drag_.a + dt, 0.0, pc.end - kMinPiece);
        else pc.end = std::clamp(drag_.b + dt, pc.start + kMinPiece, pr.srcDuration(drag_.index));
        host_->modelEdited(true);
    } else if (drag_.kind == Drag::ItemDrag) {
        Item* it = pr.item(drag_.index);
        if (!it) return;
        double dt = (pos.x() - drag_.x0) / pps(), total = pr.total();
        if (drag_.side == 'm') {
            double dur = drag_.b - drag_.a;
            it->t0 = std::clamp(drag_.a + dt, 0.0, std::max(0.0, total - dur));
            it->t1 = it->t0 + dur;
        } else if (drag_.side == 'l') {
            it->t0 = std::clamp(drag_.a + dt, 0.0, it->t1 - 0.1);
        } else {
            it->t1 = std::clamp(drag_.b + dt, it->t0 + 0.1, total);
        }
        host_->modelEdited(true);
    } else if (drag_.kind == Drag::AudioDrag) {
        AudioClip* a = pr.audio(drag_.index);
        if (!a) return;
        double dt = (pos.x() - drag_.x0) / pps();
        if (drag_.side == 'm') {
            a->t0 = std::max(0.0, drag_.a + dt);
        } else if (drag_.side == 'l') {
            double d = std::clamp(dt, -std::min(drag_.a, drag_.c), drag_.b - 0.1);
            a->t0 = drag_.a + d;
            a->srcStart = drag_.c + d;
            a->dur = drag_.b - d;
        } else {
            a->dur = std::clamp(drag_.b + dt, 0.1, a->fileDur - a->srcStart);
        }
        host_->modelEdited(true);
    }
}

void Timeline::mouseReleaseEvent(QMouseEvent*) {
    bool trimmed = drag_.kind == Drag::Trim;
    drag_.kind = Drag::None;
    if (trimmed) host_->seek(host_->curTime());
}

void Timeline::wheelEvent(QWheelEvent* e) {
    if (e->modifiers() & Qt::ControlModifier) {
        zoom_ = std::clamp(zoom_ * (e->angleDelta().y() > 0 ? 1.2 : 1 / 1.2), 1.0, 40.0);
        relayout();
        e->accept();
    } else {
        e->ignore();
    }
}
