#include "timeline.h"

#include <QFileInfo>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <cmath>
#include "i18n.h"
#include "theme.h"

static constexpr double kMinPiece = 0.1;

// Lautstärke wie in Schnittprogrammen üblich in dB: -60 dB = stumm, +6 dB = doppelt so laut
static double toDb(double v) { return v <= 0.001 ? -60.0 : std::clamp(20.0 * std::log10(v), -60.0, 6.0); }
static double fromDb(double db) { return db <= -59.9 ? 0.0 : std::pow(10.0, db / 20.0); }

Timeline::Timeline(EditorHost* host) : host_(host) {
    setMouseTracking(true);
    setMinimumHeight(RULER + ROW + LANE * 2 + 16);
}

double Timeline::basePps() const {
    const double vw = host_->timelineViewportWidth() - 2 * LEFT;
    return std::max(0.001, vw / std::max(host_->pr().total(), 0.5));
}

double Timeline::pps() const { return basePps() * zoom_; }

void Timeline::relayout() {
    Project& pr = host_->pr();
    // höchstens ~250 Pixel pro Sekunde, auch bei stundenlangen Aufnahmen
    zoom_ = std::clamp(zoom_, 1.0, std::max(1.0, 250.0 / basePps()));
    int lanes = std::max<int>(2, pr.items.size() + pr.audios.size());
    setMinimumHeight(RULER + ROW + LANE * lanes + 16);
    const double w = pr.total() * pps() + 2 * LEFT;
    setFixedWidth(int(std::min(16000000.0, std::max(w, double(host_->timelineViewportWidth())))));
    update();
}

void Timeline::zoomBy(double factor) {
    zoom_ *= factor;
    relayout();
    host_->timelineZoomed();
}

static QColor mix(const QColor& a, const QColor& b, double t) {
    return QColor::fromRgbF(a.redF() * (1 - t) + b.redF() * t, a.greenF() * (1 - t) + b.greenF() * t,
                            a.blueF() * (1 - t) + b.blueF() * t);
}

QRectF Timeline::pieceRect(int i) const {
    Project& pr = host_->pr();
    const double x0 = xOf(pr.outStart(i)), x1 = xOf(pr.outStart(i) + pr.pieces[i].outDur());
    return QRectF(x0 + 1, RULER + 2, std::max(2.0, x1 - x0 - 2), ROW - 6);
}

QRectF Timeline::waveRect(int i) const {
    const QRectF r = pieceRect(i);
    return QRectF(r.left(), r.bottom() - WAVE, r.width(), WAVE);
}

QRectF Timeline::audioRect(int k) const {
    Project& pr = host_->pr();
    const AudioClip& a = pr.audios[k];
    return QRectF(xOf(a.t0), laneY(pr.items.size() + k) + 2, std::max(6.0, a.dur * pps()), LANE - 4);
}

double Timeline::volLineY(const QRectF& area, double volume) const {
    const double frac = (toDb(volume) + 60.0) / 66.0;
    return area.bottom() - 3 - frac * (area.height() - 6);
}

void Timeline::drawWave(QPainter& p, const QRectF& area, const WaveSet* ws, double srcAtLeft, double srcPerPx,
                        double volume, bool centered, const QColor& color, const QRect& vis) const {
    if (!ws || ws->peaks.empty()) return;
    p.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::FlatCap));
    const double from = std::max(area.left() + 1, double(vis.left()));
    const double to = std::min(area.right() - 1, double(vis.right()));
    const int n = int(ws->peaks.size());
    for (double x = std::floor(from / 2) * 2; x < to; x += 2) {
        const double s0 = srcAtLeft + (x - area.left()) * srcPerPx;
        int i0 = int(s0 * ws->rate), i1 = std::max(i0 + 1, int((s0 + 2 * srcPerPx) * ws->rate));
        if (i0 >= n) break;
        if (i1 <= 0) continue;
        i0 = std::max(0, i0);
        i1 = std::min(n, i1);
        float amp = 0;
        for (int i = i0; i < i1; ++i) amp = std::max(amp, ws->peaks[i]);
        const double h = std::min(1.0, std::pow(amp * volume, 0.7)) * (area.height() - 4);
        if (h < 0.6) continue;
        if (centered) p.drawLine(QPointF(x, area.center().y() - h / 2), QPointF(x, area.center().y() + h / 2));
        else p.drawLine(QPointF(x, area.bottom() - 2), QPointF(x, area.bottom() - 2 - h));
    }
}

void Timeline::drawDbPill(QPainter& p, double x, double y, double volume) const {
    const double db = toDb(volume);
    const QString t = db <= -59.9 ? QStringLiteral("-∞ dB") : QString("%1%2 dB").arg(db > 0.05 ? "+" : "").arg(db, 0, 'f', 1);
    QFont f = font();
    f.setPixelSize(11);
    f.setBold(true);
    p.setFont(f);
    const QFontMetrics fm(f);
    QRectF pill(x - fm.horizontalAdvance(t) / 2.0 - 7, y - 24, fm.horizontalAdvance(t) + 14, 18);
    pill.moveLeft(std::clamp(pill.left(), 2.0, width() - pill.width() - 2));
    pill.moveTop(std::max(1.0, pill.top()));
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(20, 20, 24, 235));
    p.drawRoundedRect(pill, 6, 6);
    p.setPen(Qt::white);
    p.drawText(pill, Qt::AlignCenter, t);
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
    double step = 3600;
    for (double s : {0.1, 0.25, 0.5, 1.0, 2.0, 5.0, 10.0, 15.0, 30.0, 60.0, 120.0, 300.0, 600.0, 1200.0, 1800.0})
        if (s * sc >= 70) { step = s; break; }
    QFont small = font();
    small.setPixelSize(11);
    p.setFont(small);
    const double tFrom = std::max(0.0, std::floor(tOf(vis.left() - 80) / step) * step);
    for (double t = tFrom; t <= total + 0.001 && xOf(t) < vis.right() + 80; t += step) {
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

    // Video-Abschnitte: Vorschaubilder oben, Ton-Wellenform mit Lautstärke-Linie unten
    const QColor pieceBase = mix(th.panel2, th.accent, 0.35);
    const QColor waveBg = mix(th.timeline, th.accent, 0.22);
    const QColor waveFg = mix(th.accent, QColor(Qt::white), 0.25);
    for (int i = 0; i < pr.pieces.size(); ++i) {
        const Piece& pc = pr.pieces[i];
        const QRectF r = pieceRect(i);
        if (r.right() < vis.left() - 4 || r.left() > vis.right() + 4) continue;
        const QRectF rt(r.left(), r.top(), r.width(), r.height() - WAVE);
        const QRectF rw = waveRect(i);
        const bool sel = i == host_->selPiece();
        QPainterPath clip;
        clip.addRoundedRect(r, 7, 7);
        p.save();
        p.setClipPath(clip);
        p.fillRect(r, pieceBase);
        if (const ThumbSet* ts = host_->thumbs(pc.src); ts && !ts->imgs.isEmpty()) {
            const QImage& first = ts->imgs.first();
            const double tw = rt.height() * first.width() / std::max(1, first.height());
            const double from = std::max(rt.left(), double(vis.left()) - tw);
            const double to = std::min(rt.right(), double(vis.right()) + tw);
            for (double tx = rt.left() + std::floor((from - rt.left()) / tw) * tw; tx < to; tx += tw) {
                double tOut = (tx + tw / 2 - rt.left()) / sc;
                double src = pc.start + std::clamp(tOut, 0.0, pc.outDur()) * pc.speed;
                int idx = std::clamp(int(src / ts->step), 0, int(ts->imgs.size()) - 1);
                p.drawImage(QRectF(tx, rt.top(), tw + 1, rt.height()), ts->imgs[idx]);
            }
        }
        QLinearGradient shade(0, rt.top(), 0, rt.bottom());
        shade.setColorAt(0, QColor(0, 0, 0, 10));
        shade.setColorAt(1, QColor(0, 0, 0, 120));
        p.fillRect(rt, shade);
        p.fillRect(rw, waveBg);
        drawWave(p, rw, host_->waves(pr.sources.value(pc.src).path), pc.start, pc.speed / sc, pc.volume, false,
                 waveFg, vis);
        // Lautstärke-Linie
        const double ly = volLineY(rw, pc.volume);
        const bool hot = (hover_ == Hover::Piece && hoverIndex_ == i) || (drag_.kind == Drag::PieceVol && drag_.index == i);
        p.setPen(QPen(QColor(255, 255, 255, hot ? 255 : 170), hot ? 2 : 1.3));
        p.drawLine(QPointF(rw.left() + 2, ly), QPointF(rw.right() - 2, ly));
        if (sel) p.fillRect(r, QColor(th.accent.red(), th.accent.green(), th.accent.blue(), 40));
        p.restore();
        p.setBrush(Qt::NoBrush);
        p.setPen(sel ? QPen(th.accent, 2.5) : QPen(QColor(0, 0, 0, 90), 1));
        p.drawRoundedRect(r, 7, 7);
        // Griffe zum Kürzen (an jedem Abschnitt)
        p.setPen(Qt::NoPen);
        p.setBrush(sel ? th.accent : QColor(255, 255, 255, 150));
        p.drawRoundedRect(QRectF(r.left() - 1, rt.center().y() - 11, 4, 22), 2, 2);
        p.drawRoundedRect(QRectF(r.right() - 3, rt.center().y() - 11, 4, 22), 2, 2);
        QString label = fmtTime(pc.outDur());
        if (pc.speed != 1.0) label += QString("  ×%1").arg(pc.speed, 0, 'g', 3);
        QFontMetrics fm(small);
        p.setFont(small);
        QRectF pill(r.left() + 8, rt.top() + 5, fm.horizontalAdvance(label) + 12, 16);
        if (pill.right() < r.right() - 6) {
            p.setBrush(QColor(0, 0, 0, 150));
            p.drawRoundedRect(pill, 8, 8);
            p.setPen(Qt::white);
            p.drawText(pill, Qt::AlignCenter, label);
        }
    }

    // Spuren: erst Elemente (Bild/Text/Unschärfe), dann Ton
    int lane = 0;
    for (const Item& it : pr.items) {
        QRectF r(xOf(it.t0), laneY(lane) + 2, std::max(6.0, (it.t1 - it.t0) * sc), LANE - 4);
        ++lane;
        bool sel = host_->selItem() == it.id;
        QColor base = it.kind == Item::Blur ? blurColor() : it.kind == Item::Text ? textItemColor() : imageColor();
        p.setBrush(sel ? base.lighter(125) : base);
        p.setPen(sel ? QPen(th.accent, 2) : QPen(Qt::NoPen));
        p.drawRoundedRect(r, 6, 6);
        p.setPen(Qt::white);
        p.setFont(small);
        const QString name = it.kind == Item::Blur ? T("blur")
                             : it.kind == Item::Text ? it.text.section('\n', 0, 0)
                                                     : QFileInfo(it.path).fileName();
        p.drawText(r.adjusted(8, 0, -4, 0), Qt::AlignVCenter, name);
    }
    for (int k = 0; k < pr.audios.size(); ++k) {
        const AudioClip& a = pr.audios[k];
        const QRectF r = audioRect(k);
        bool sel = host_->selAudio() == a.id;
        QColor base = audioColor();
        p.setBrush(sel ? base.lighter(118) : base.darker(115));
        p.setPen(sel ? QPen(th.accent, 2) : QPen(Qt::NoPen));
        p.drawRoundedRect(r, 6, 6);
        p.save();
        p.setClipRect(r);
        drawWave(p, r, host_->waves(a.path), a.srcStart, 1.0 / sc, a.volume, true, QColor(255, 255, 255, 120), vis);
        const double ly = volLineY(r, a.volume);
        const bool hot = (hover_ == Hover::Audio && hoverIndex_ == a.id) || (drag_.kind == Drag::AudioVol && drag_.index == a.id);
        p.setPen(QPen(QColor(255, 255, 255, hot ? 255 : 170), hot ? 2 : 1.3));
        p.drawLine(QPointF(r.left() + 2, ly), QPointF(r.right() - 2, ly));
        p.setPen(Qt::white);
        p.setFont(small);
        p.drawText(r.adjusted(8, 1, -4, 0), Qt::AlignTop | Qt::AlignLeft, QFileInfo(a.path).fileName());
        p.restore();
    }

    // dB-Anzeige an der Lautstärke-Linie
    if (drag_.kind == Drag::PieceVol && drag_.index < pr.pieces.size())
        drawDbPill(p, hoverX_, volLineY(waveRect(drag_.index), pr.pieces[drag_.index].volume), pr.pieces[drag_.index].volume);
    else if (drag_.kind == Drag::AudioVol || hover_ == Hover::Audio) {
        const int id = drag_.kind == Drag::AudioVol ? drag_.index : hoverIndex_;
        for (int k = 0; k < pr.audios.size(); ++k)
            if (pr.audios[k].id == id) drawDbPill(p, hoverX_, volLineY(audioRect(k), pr.audios[k].volume), pr.audios[k].volume);
    } else if (hover_ == Hover::Piece && hoverIndex_ >= 0 && hoverIndex_ < pr.pieces.size()) {
        drawDbPill(p, hoverX_, volLineY(waveRect(hoverIndex_), pr.pieces[hoverIndex_].volume), pr.pieces[hoverIndex_].volume);
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

AudioClip* Timeline::audioAt(const QPointF& pos, int* lane) {
    Project& pr = host_->pr();
    for (int k = 0; k < pr.audios.size(); ++k) {
        AudioClip& a = pr.audios[k];
        int y = laneY(pr.items.size() + k);
        if (pos.y() >= y && pos.y() < y + LANE && pos.x() >= xOf(a.t0) - 4 && pos.x() <= xOf(a.t0 + a.dur) + 4) {
            if (lane) *lane = k;
            return &a;
        }
    }
    return nullptr;
}

// Rand eines beliebigen Abschnitts unter der Maus (an der Grenze zweier Abschnitte zählt die Seite der Maus)
int Timeline::pieceEdgeAt(const QPointF& pos, char* side) const {
    Project& pr = host_->pr();
    if (pos.y() < RULER || pos.y() >= RULER + ROW) return -1;
    for (int i = 0; i < pr.pieces.size(); ++i) {
        const double x0 = xOf(pr.outStart(i)), x1 = xOf(pr.outStart(i) + pr.pieces[i].outDur());
        if (pos.x() >= x0 - 3 && pos.x() - x0 <= 7 && pos.x() < x1) { *side = 'l'; return i; }
        if (pos.x() <= x1 && x1 - pos.x() <= 7 && pos.x() > x0) { *side = 'r'; return i; }
    }
    return -1;
}

int Timeline::pieceVolAt(const QPointF& pos) const {
    Project& pr = host_->pr();
    for (int i = 0; i < pr.pieces.size(); ++i) {
        const QRectF rw = waveRect(i);
        if (pos.x() < rw.left() || pos.x() > rw.right()) continue;
        return std::abs(pos.y() - volLineY(rw, pr.pieces[i].volume)) <= 5 ? i : -1;
    }
    return -1;
}

AudioClip* Timeline::audioVolAt(const QPointF& pos) {
    int k = -1;
    AudioClip* a = audioAt(pos, &k);
    if (!a) return nullptr;
    const QRectF r = audioRect(k);
    return std::abs(pos.y() - volLineY(r, a->volume)) <= 5 ? a : nullptr;
}

void Timeline::mousePressEvent(QMouseEvent* e) {
    const QPointF pos = e->position();
    Project& pr = host_->pr();
    if (pr.pieces.isEmpty()) return;
    hoverX_ = pos.x();

    if (pos.y() < RULER) {
        drag_.kind = Drag::Seek;
        host_->seek(tOf(pos.x()));
        return;
    }
    if (pos.y() < RULER + ROW) {
        char side = 0;
        const int edge = pieceEdgeAt(pos, &side);
        if (edge >= 0) {
            const Piece& pc = pr.pieces[edge];
            host_->pushUndo();
            host_->selectPiece(edge);
            drag_ = {Drag::Trim, edge, side, pos.x(), pc.start, pc.end, 0, pos.y()};
            return;
        }
        const int vol = pieceVolAt(pos);
        if (vol >= 0) {
            host_->pushUndo();
            host_->selectPiece(vol);
            drag_ = {Drag::PieceVol, vol, 0, pos.x(), toDb(pr.pieces[vol].volume), 0, 0, pos.y()};
            update();
            return;
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
        drag_ = {Drag::ItemDrag, it->id, mode, pos.x(), it->t0, it->t1, 0, pos.y()};
    } else if (AudioClip* a = audioAt(pos)) {
        host_->selectAudio(a->id);
        double x0 = xOf(a->t0), x1 = xOf(a->t0 + a->dur);
        char mode = std::abs(pos.x() - x0) <= 6 ? 'l' : std::abs(pos.x() - x1) <= 6 ? 'r' : 'm';
        host_->pushUndo();
        if (mode == 'm' && audioVolAt(pos))
            drag_ = {Drag::AudioVol, a->id, 0, pos.x(), toDb(a->volume), 0, 0, pos.y()};
        else
            drag_ = {Drag::AudioDrag, a->id, mode, pos.x(), a->t0, a->dur, a->srcStart, pos.y()};
    } else {
        host_->selectItem(0);
        host_->selectAudio(0);
        host_->selectPiece(-1);
    }
}

void Timeline::mouseMoveEvent(QMouseEvent* e) {
    const QPointF pos = e->position();
    Project& pr = host_->pr();
    hoverX_ = pos.x();
    if (drag_.kind == Drag::None) {
        char side = 0;
        bool near = pieceEdgeAt(pos, &side) >= 0;
        if (Item* it = itemAt(pos)) near |= std::abs(pos.x() - xOf(it->t0)) <= 6 || std::abs(pos.x() - xOf(it->t1)) <= 6;
        if (AudioClip* a = audioAt(pos)) near |= std::abs(pos.x() - xOf(a->t0)) <= 6 || std::abs(pos.x() - xOf(a->t0 + a->dur)) <= 6;
        Hover h = Hover::None;
        int idx = -1;
        if (!near) {
            if ((idx = pieceVolAt(pos)) >= 0) h = Hover::Piece;
            else if (AudioClip* a = audioVolAt(pos)) { h = Hover::Audio; idx = a->id; }
        }
        setCursor(near ? Qt::SizeHorCursor : h != Hover::None ? Qt::SizeVerCursor : Qt::ArrowCursor);
        if (h != hover_ || idx != hoverIndex_ || h != Hover::None) {
            hover_ = h;
            hoverIndex_ = idx;
            update();
        }
        return;
    }
    if (drag_.kind == Drag::Seek) {
        host_->scrub(tOf(pos.x()));
    } else if (drag_.kind == Drag::Trim) {
        if (drag_.index >= pr.pieces.size()) return;
        Piece& pc = pr.pieces[drag_.index];
        double dt = (pos.x() - drag_.x0) / pps() * pc.speed;
        if (drag_.side == 'l') pc.start = std::clamp(drag_.a + dt, 0.0, pc.end - kMinPiece);
        else pc.end = std::clamp(drag_.b + dt, pc.start + kMinPiece, pr.srcDuration(drag_.index));
        host_->modelEdited(true);
    } else if (drag_.kind == Drag::PieceVol || drag_.kind == Drag::AudioVol) {
        // nach oben lauter, nach unten leiser (feiner mit gedrückter Umschalttaste)
        const double perPx = (e->modifiers() & Qt::ShiftModifier) ? 0.08 : 0.35;
        const double v = fromDb(std::clamp(drag_.a - (pos.y() - drag_.y0) * perPx, -60.0, 6.0));
        if (drag_.kind == Drag::PieceVol) {
            if (drag_.index < pr.pieces.size()) pr.pieces[drag_.index].volume = v;
        } else if (AudioClip* a = pr.audio(drag_.index)) {
            a->volume = v;
        }
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
    bool settle = drag_.kind == Drag::Trim || drag_.kind == Drag::Seek;
    drag_.kind = Drag::None;
    if (settle) host_->seek(host_->curTime());
    update();
}

void Timeline::leaveEvent(QEvent*) {
    if (hover_ != Hover::None) {
        hover_ = Hover::None;
        update();
    }
}

void Timeline::wheelEvent(QWheelEvent* e) {
    if (e->modifiers() & Qt::ControlModifier) {
        zoomBy(e->angleDelta().y() > 0 ? 1.25 : 1 / 1.25);
        e->accept();
    } else {
        e->ignore();
    }
}
