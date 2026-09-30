#include "mainwindow.h"

#include <QAction>
#include <QDateTime>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QProgressDialog>
#include <QSettings>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoSink>
#include "dialogs.h"
#include "i18n.h"
#include "theme.h"

static constexpr double kMinPiece = 0.1;

MainWindow::MainWindow() {
    resize(1280, 820);
    setMinimumSize(900, 600);
    setAcceptDrops(true);
    ambientClock_.start();
    buildUi();
    applyTheme();
    retranslate();
    setEditMode(false);
}

// ---------------------------------------------------------------- Aufbau
QPushButton* MainWindow::mk(const char* textKey, const char* tipKey, Ic ic, const char* objName,
                            const char* tipSuffix) {
    auto* b = new QPushButton;
    if (objName) b->setObjectName(objName);
    b->setFocusPolicy(Qt::NoFocus);
    b->setCursor(Qt::PointingHandCursor);
    b->setIconSize(QSize(18, 18));
    const bool accent = objName && (QString(objName) == "primary" || QString(objName) == "play");
    btns_.push_back({b, textKey, tipKey, ic, tipSuffix, accent});
    return b;
}

QWidget* MainWindow::buildInspector() {
    insp_ = new QStackedWidget;

    // 0: Hinweis
    auto* hint = new QWidget;
    auto* hv = new QVBoxLayout(hint);
    lblInspHint_ = new QLabel;
    lblInspHint_->setObjectName("hint");
    lblInspHint_->setWordWrap(true);
    hv->addWidget(lblInspHint_);
    hv->addStretch();
    insp_->addWidget(hint);

    // 1: Abschnitt
    auto* pg = new QWidget;
    auto* v = new QVBoxLayout(pg);
    v->setSpacing(10);
    lblSegment_ = new QLabel;
    lblSegment_->setObjectName("title");
    v->addWidget(lblSegment_);
    lblSpeed_ = new QLabel;
    v->addWidget(lblSpeed_);
    auto* row = new QHBoxLayout;
    for (double s : {0.5, 1.0, 2.0, 4.0}) {
        auto* b = new QPushButton(QString("%1×").arg(s));
        b->setFocusPolicy(Qt::NoFocus);
        connect(b, &QPushButton::clicked, this, [this, s] { setSpeed(s); });
        row->addWidget(b);
    }
    v->addLayout(row);
    spSpeed_ = new QDoubleSpinBox;
    spSpeed_->setRange(0.1, 16);
    spSpeed_->setSingleStep(0.25);
    spSpeed_->setDecimals(2);
    spSpeed_->setSuffix(" ×");
    spSpeed_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    connect(spSpeed_, &QDoubleSpinBox::editingFinished, this, [this] { setSpeed(spSpeed_->value()); });
    v->addWidget(spSpeed_);
    lblPieceVol_ = new QLabel;
    v->addWidget(lblPieceVol_);
    sliderPieceVol_ = new QSlider(Qt::Horizontal);
    sliderPieceVol_->setRange(0, 200);
    sliderPieceVol_->setFocusPolicy(Qt::NoFocus);
    connect(sliderPieceVol_, &QSlider::sliderPressed, this, &MainWindow::pushUndo);
    connect(sliderPieceVol_, &QSlider::valueChanged, this, &MainWindow::setPieceVolume);
    v->addWidget(sliderPieceVol_);
    lblPiece_ = new QLabel;
    lblPiece_->setObjectName("hint");
    lblPiece_->setWordWrap(true);
    v->addWidget(lblPiece_);
    v->addStretch();
    insp_->addWidget(pg);

    // 2: Element (Bild / Unschärfe / Ton)
    pg = new QWidget;
    v = new QVBoxLayout(pg);
    v->setSpacing(10);
    lblItem_ = new QLabel;
    lblItem_->setObjectName("title");
    v->addWidget(lblItem_);
    spT0_ = new QDoubleSpinBox;
    spT1_ = new QDoubleSpinBox;
    lblStart_ = new QLabel;
    lblEnd_ = new QLabel;
    int n = 0;
    for (QDoubleSpinBox* sp : {spT0_, spT1_}) {
        sp->setRange(0, 36000);
        sp->setDecimals(2);
        sp->setSingleStep(0.25);
        sp->setButtonSymbols(QAbstractSpinBox::NoButtons);
        connect(sp, &QDoubleSpinBox::editingFinished, this, &MainWindow::itemTimesChanged);
        v->addWidget(n++ == 0 ? lblStart_ : lblEnd_);
        v->addWidget(sp);
    }
    lblStr_ = new QLabel;
    v->addWidget(lblStr_);
    sliderStr_ = new QSlider(Qt::Horizontal);
    sliderStr_->setRange(5, 100);
    sliderStr_->setFocusPolicy(Qt::NoFocus);
    connect(sliderStr_, &QSlider::sliderPressed, this, &MainWindow::pushUndo);
    connect(sliderStr_, &QSlider::valueChanged, this, &MainWindow::sliderChanged);
    v->addWidget(sliderStr_);
    btnDelEl_ = mk("delete_el", nullptr, Ic::Trash);
    connect(btnDelEl_, &QPushButton::clicked, this, &MainWindow::deleteSelected);
    v->addWidget(btnDelEl_);
    v->addStretch();
    insp_->addWidget(pg);

    inspCard_ = new QFrame;
    inspCard_->setObjectName("card");
    inspCard_->setFixedWidth(260);
    auto* cl = new QVBoxLayout(inspCard_);
    cl->setContentsMargins(14, 14, 14, 14);
    cl->addWidget(insp_);
    return inspCard_;
}

void MainWindow::buildUi() {
    // Player + Videoansicht
    player_ = new QMediaPlayer(this);
    audio_ = new QAudioOutput(this);
    audio_->setVolume(float(masterVol_));
    player_->setAudioOutput(audio_);
    scene_ = new QGraphicsScene(this);
    canvasBg_ = scene_->addRect(0, 0, 1920, 1080, Qt::NoPen, Qt::black);
    canvasBg_->setVisible(false);  // erst mit geladenem Video (sonst sieht man den leeren Hintergrund)
    canvasBg_->setZValue(-1);
    vitem_ = new QGraphicsVideoItem;
    vitem_->setZValue(0);
    scene_->addItem(vitem_);
    player_->setVideoOutput(vitem_);
    view_ = new VideoView(scene_);
    connect(vitem_, &QGraphicsVideoItem::nativeSizeChanged, this, &MainWindow::onNativeSize);
    connect(vitem_->videoSink(), &QVideoSink::videoFrameChanged, this, &MainWindow::onFrame);
    connect(player_, &QMediaPlayer::durationChanged, this, &MainWindow::onDuration);
    connect(player_, &QMediaPlayer::playbackStateChanged, this, &MainWindow::onState);
    connect(player_, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::onStatus);
    connect(view_, &VideoView::emptyClicked, this, [this] { pr_.pieces.isEmpty() ? openDialog() : togglePlay(); });
    timer_ = new QTimer(this);
    timer_->setInterval(15);
    connect(timer_, &QTimer::timeout, this, &MainWindow::tick);
    cursorTimer_ = new QTimer(this);
    cursorTimer_->setSingleShot(true);
    cursorTimer_->setInterval(1800);
    connect(cursorTimer_, &QTimer::timeout, this, [this] {
        if (fullscreen_) {
            view_->viewport()->setCursor(Qt::BlankCursor);
            fsBtn_->hide();
        }
    });
    connect(view_, &VideoView::mouseActivity, this, [this] {
        view_->viewport()->unsetCursor();
        fsBtn_->show();
        if (fullscreen_) cursorTimer_->start();
    });

    // Obere Leiste
    topbar_ = new QWidget;
    topbar_->setObjectName("topbar");
    auto* top = new QHBoxLayout(topbar_);
    top->setContentsMargins(0, 0, 0, 0);
    top->setSpacing(8);
    auto* bOpen = mk("open", "open_video", Ic::Open, nullptr, " (Ctrl+O)");
    auto* bExp = mk("export", nullptr, Ic::Export, "primary", " (Ctrl+E)");
    connect(bOpen, &QPushButton::clicked, this, &MainWindow::openDialog);
    connect(bExp, &QPushButton::clicked, this, &MainWindow::exportVideo);
    top->addWidget(bOpen);
    top->addStretch();
    top->addWidget(bExp);

    // Transportleiste
    transport_ = new QWidget;
    auto* bar = new QHBoxLayout(transport_);
    bar->setContentsMargins(0, 0, 0, 0);
    bar->setSpacing(12);
    btnPlay_ = mk(nullptr, "play_tip", Ic::Play, "play");
    connect(btnPlay_, &QPushButton::clicked, this, &MainWindow::togglePlay);
    slider_ = new SeekSlider;
    slider_->setRange(0, 10000);
    slider_->setFocusPolicy(Qt::NoFocus);
    connect(slider_, &QSlider::sliderMoved, this, [this](int v) {
        showScrubPreview(v);
        scrub(v / 10000.0 * pr_.total());
    });
    connect(slider_, &QSlider::sliderReleased, this, [this] {
        scrubPrev_->hide();
        seek(slider_->value() / 10000.0 * pr_.total());
    });
    seekTimer_ = new QTimer(this);
    seekTimer_->setSingleShot(true);
    connect(seekTimer_, &QTimer::timeout, this, [this] {
        seekThrottle_.restart();
        seek(scrubTarget_);
    });
    lblTime_ = new QLabel("00:00.0 / 00:00.0");
    lblVolIcon_ = new QLabel;
    auto* vol = new QSlider(Qt::Horizontal);
    vol->setFixedWidth(90);
    vol->setRange(0, 100);
    vol->setValue(int(masterVol_ * 100));
    vol->setFocusPolicy(Qt::NoFocus);
    connect(vol, &QSlider::valueChanged, this, [this](int v) {
        masterVol_ = v / 100.0;
        applyPlayerVolume();
        updateAudio(false);
    });
    btnAddMedia_ = mk("add_media", "add_media_tip", Ic::AddVideo);
    connect(btnAddMedia_, &QPushButton::clicked, this, &MainWindow::addMedia);
    btnEdit_ = mk("edit", nullptr, Ic::Edit, nullptr, " (E)");
    btnEdit_->setCheckable(true);
    connect(btnEdit_, &QPushButton::toggled, this, &MainWindow::setEditMode);
    auto* bSet = mk(nullptr, "settings", Ic::Settings);
    connect(bSet, &QPushButton::clicked, this, &MainWindow::openSettings);
    bar->addWidget(btnPlay_);
    bar->addWidget(slider_, 1);
    bar->addWidget(lblTime_);
    bar->addWidget(lblVolIcon_);
    bar->addWidget(vol);
    bar->addWidget(btnAddMedia_);
    bar->addWidget(btnEdit_);
    bar->addWidget(bSet);

    // Werkzeuge
    tools_ = new QWidget;
    auto* tl = new QHBoxLayout(tools_);
    tl->setContentsMargins(0, 0, 0, 0);
    tl->setSpacing(8);
    struct T { const char* text; const char* tip; Ic ic; const char* suffix; void (MainWindow::*fn)(); };
    const T list[] = {
        {"split", nullptr, Ic::Scissors, " (S)", &MainWindow::doSplit},
        {"remove_piece", nullptr, Ic::Trash, " (Del)", &MainWindow::doDelete},
        {"trim_start", nullptr, Ic::TrimStart, " (Q)", &MainWindow::doTrimStart},
        {"trim_end", nullptr, Ic::TrimEnd, " (W)", &MainWindow::doTrimEnd},
        {"blur", nullptr, Ic::Blur, " (B)", &MainWindow::addBlur},
        {"image", nullptr, Ic::Image, " (I)", &MainWindow::addImage},
        {nullptr, "undo", Ic::Undo, "", &MainWindow::doUndo},
        {nullptr, "redo", Ic::Redo, "", &MainWindow::doRedo},
    };
    for (const T& t : list) {
        auto* b = mk(t.text, t.tip, t.ic, "tool", t.suffix);
        connect(b, &QPushButton::clicked, this, [this, fn = t.fn] { (this->*fn)(); });
        tl->addWidget(b);
    }
    tl->addStretch();

    // Timeline
    timeline_ = new Timeline(this);
    tlScroll_ = new QScrollArea;
    tlScroll_->setWidget(timeline_);
    tlScroll_->setWidgetResizable(false);
    tlScroll_->setFixedHeight(210);
    tlScroll_->viewport()->installEventFilter(this);

    auto* mid = new QHBoxLayout;
    mid->setSpacing(12);
    mid->addWidget(view_, 1);
    mid->addWidget(buildInspector());
    auto* root = new QWidget;
    root->setObjectName("root");
    rootLayout_ = new QVBoxLayout(root);
    rootLayout_->setContentsMargins(14, 12, 14, 14);
    rootLayout_->setSpacing(10);
    rootLayout_->addWidget(topbar_);
    rootLayout_->addLayout(mid, 1);
    rootLayout_->addWidget(transport_);
    rootLayout_->addWidget(tools_);
    rootLayout_->addWidget(tlScroll_);
    setCentralWidget(root);

    // Vollbild-Knopf direkt im Video (unten rechts)
    fsBtn_ = new QPushButton(view_);
    fsBtn_->setObjectName("vidbtn");
    fsBtn_->setFocusPolicy(Qt::NoFocus);
    fsBtn_->setCursor(Qt::PointingHandCursor);
    fsBtn_->setFixedSize(40, 40);
    fsBtn_->setIconSize(QSize(18, 18));
    fsBtn_->setStyleSheet("QPushButton#vidbtn { background: rgba(0,0,0,0.45); border: 1px solid rgba(255,255,255,0.25); border-radius: 10px; }"
                          "QPushButton#vidbtn:hover { background: rgba(0,0,0,0.7); }");
    connect(fsBtn_, &QPushButton::clicked, this, &MainWindow::toggleFullscreen);
    fsHint_ = new QLabel(view_);
    fsHint_->setAlignment(Qt::AlignCenter);
    fsHint_->setStyleSheet("color:#ffffff;background:rgba(0,0,0,0.65);border-radius:12px;padding:10px 18px;font-size:15px;font-weight:600;");
    fsHint_->hide();
    view_->installEventFilter(this);

    scrubPrev_ = new QLabel(this);
    scrubPrev_->setAttribute(Qt::WA_TransparentForMouseEvents);
    scrubPrev_->hide();

    lblHint_ = new QLabel(view_);
    lblHint_->setStyleSheet("color:#8b90a0;font-size:17px;background:transparent");

    addShortcut(Qt::Key_Space, [this] { togglePlay(); });
    addShortcut(QKeySequence("Ctrl+O"), [this] { openDialog(); });
    addShortcut(QKeySequence("Ctrl+E"), [this] { exportVideo(); });
    addShortcut(Qt::Key_E, [this] { btnEdit_->toggle(); });
    addShortcut(Qt::Key_F11, [this] { toggleFullscreen(); });
    addShortcut(Qt::Key_Escape, [this] { if (fullscreen_) exitFullscreen(); });
    addShortcut(Qt::Key_S, [this] { doSplit(); });
    addShortcut(Qt::Key_Delete, [this] { doDelete(); });
    addShortcut(Qt::Key_Q, [this] { doTrimStart(); });
    addShortcut(Qt::Key_W, [this] { doTrimEnd(); });
    addShortcut(Qt::Key_B, [this] { addBlur(); });
    addShortcut(Qt::Key_I, [this] { addImage(); });
    addShortcut(QKeySequence("Ctrl+Z"), [this] { doUndo(); });
    addShortcut(QKeySequence("Ctrl+Y"), [this] { doRedo(); });
    addShortcut(QKeySequence("Ctrl+Shift+Z"), [this] { doRedo(); });
    addShortcut(Qt::Key_Left, [this] { seek(t_ - 5.0); });
    addShortcut(Qt::Key_Right, [this] { seek(t_ + 5.0); });
}

void MainWindow::addShortcut(const QKeySequence& k, std::function<void()> fn) {
    auto* a = new QAction(this);
    a->setShortcut(k);
    connect(a, &QAction::triggered, this, [fn] { fn(); });
    addAction(a);
}

void MainWindow::applyTheme() {
    const Theme& th = currentTheme();
    qApp->setStyleSheet(buildStyleSheet(th));
    for (const BtnSpec& s : btns_) {
        if (s.b == btnPlay_) continue;
        s.b->setProperty("dummy", 0);
        const bool onBar = th.xp && topbar_->isAncestorOf(s.b);  // XP-Taskleiste: weiße Symbole
        static_cast<QPushButton*>(s.b)->setIcon(makeIcon(s.icon, s.onAccent || onBar ? th.accentText : th.text));
    }
    lblVolIcon_->setPixmap(makeIcon(Ic::Volume, th.muted).pixmap(20, 20));
    lblHint_->setStyleSheet(th.xp ? QString("color:#FFFFFF;font-size:17px;font-weight:bold;background:transparent")
                                  : QString("color:%1;font-size:17px;background:transparent").arg(th.muted.name()));
    // XP: Taskleiste mit Innenabstand, sonst bündig
    if (auto* l = topbar_->layout()) l->setContentsMargins(th.xp ? QMargins(6, 4, 6, 4) : QMargins(0, 0, 0, 0));
    updatePlayIcon();
    view_->viewport()->update();
    timeline_->update();
    for (Overlay* ov : overlays_) ov->update();
}

void MainWindow::updatePlayIcon() {
    const Theme& th = currentTheme();
    bool playing = player_->playbackState() == QMediaPlayer::PlayingState;
    btnPlay_->setIcon(makeIcon(playing ? Ic::Pause : Ic::Play, th.accentText));
    btnPlay_->setIconSize(QSize(20, 20));
}

void MainWindow::retranslate() {
    setWindowTitle(pr_.sources.isEmpty() ? QString("Cutline")
                                         : "Cutline – " + QFileInfo(pr_.sources[0].path).fileName());
    for (const BtnSpec& s : btns_) {
        QString text = s.text ? T(s.text) : QString();
        s.b->setText(text.isEmpty() ? QString() : " " + text);
        QString tip = s.tip ? T(s.tip) : (s.text ? T(s.text) : QString());
        s.b->setToolTip(tip + QString::fromLatin1(s.tipSuffix));
    }
    lblHint_->setText(T("drop_hint"));
    lblHint_->adjustSize();
    lblInspHint_->setText(T("insp_hint"));
    lblSegment_->setText(T("segment"));
    lblSpeed_->setText(T("speed"));
    lblStart_->setText(T("start_s"));
    lblEnd_->setText(T("end_s"));
    lblPieceVol_->setText(T("volume"));
    lblHint_->move((view_->width() - lblHint_->width()) / 2, view_->height() / 2 - 10);
    refreshInspector();
    timeline_->update();
}

void MainWindow::placeVideoControls() {
    if (!fsBtn_) return;
    fsBtn_->setIcon(makeIcon(fullscreen_ ? Ic::ExitFullscreen : Ic::Fullscreen, Qt::white));
    fsBtn_->setToolTip(T("fullscreen_tip"));
    fsBtn_->move(view_->width() - fsBtn_->width() - 14, view_->height() - fsBtn_->height() - 14);
    fsBtn_->raise();
    if (fsHint_->isVisible()) {
        fsHint_->adjustSize();
        fsHint_->move((view_->width() - fsHint_->width()) / 2, 40);
    }
}

// Beim Wechsel ins Vollbild kurz zeigen, wie man wieder herauskommt
void MainWindow::showFullscreenHint() {
    fsHint_->setText(T("fs_exit_hint"));
    fsHint_->adjustSize();
    fsHint_->move((view_->width() - fsHint_->width()) / 2, 40);
    fsHint_->show();
    fsHint_->raise();
    auto* eff = new QGraphicsOpacityEffect(fsHint_);
    eff->setOpacity(1.0);
    fsHint_->setGraphicsEffect(eff);
    auto* anim = new QPropertyAnimation(eff, "opacity", fsHint_);
    anim->setDuration(700);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    QTimer::singleShot(2800, anim, [this, anim] {
        connect(anim, &QPropertyAnimation::finished, fsHint_, &QWidget::hide);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

bool MainWindow::eventFilter(QObject* o, QEvent* e) {
    if (o == view_ && e->type() == QEvent::Resize) placeVideoControls();
    if (o == tlScroll_->viewport() && e->type() == QEvent::Resize)
        QTimer::singleShot(0, this, [this] { timeline_->relayout(); });
    return QMainWindow::eventFilter(o, e);
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    fitView();
    lblHint_->move((view_->width() - lblHint_->width()) / 2, view_->height() / 2 - 10);
}

void MainWindow::closeEvent(QCloseEvent* e) {
    if (exportProc_) exportProc_->kill();
    QMainWindow::closeEvent(e);
}

void MainWindow::fitView() {
    if (scene_->sceneRect().width() > 0) {
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
        view_->viewport()->update();
    }
}

// Video (evtl. mit anderer Größe als die Bildfläche) mittig einpassen
void MainWindow::fitVideoItem() {
    QSizeF s = vitem_->nativeSize();
    if (!s.isValid() || s.isEmpty()) return;
    double k = std::min(double(pr_.vw) / s.width(), double(pr_.vh) / s.height());
    QSizeF fs(s.width() * k, s.height() * k);
    vitem_->setSize(fs);
    vitem_->setPos((pr_.vw - fs.width()) / 2, (pr_.vh - fs.height()) / 2);
}

// ---------------------------------------------------------------- Modi / Vollbild
void MainWindow::setEditMode(bool on) {
    Q_UNUSED(on);
    applyLayoutVisibility();
    for (Overlay* ov : overlays_) ov->setAcceptedMouseButtons(showGuides() ? Qt::LeftButton : Qt::NoButton);
    for (Overlay* ov : overlays_) ov->update();
}

void MainWindow::applyLayoutVisibility() {
    const bool edit = btnEdit_->isChecked();
    topbar_->setVisible(!fullscreen_ && edit);  // obere Leiste nur beim Bearbeiten
    transport_->setVisible(!fullscreen_);
    tools_->setVisible(!fullscreen_ && edit);
    btnAddMedia_->setVisible(edit);
    tlScroll_->setVisible(!fullscreen_ && edit);
    inspCard_->setVisible(!fullscreen_ && edit);
    if (fullscreen_) {
        rootLayout_->setContentsMargins(0, 0, 0, 0);
        rootLayout_->setSpacing(0);
    } else {
        rootLayout_->setContentsMargins(14, 12, 14, 14);
        rootLayout_->setSpacing(10);
    }
    QTimer::singleShot(0, this, [this] { fitView(); timeline_->relayout(); });
}

void MainWindow::toggleFullscreen() { fullscreen_ ? exitFullscreen() : enterFullscreen(); }

void MainWindow::enterFullscreen() {
    fullscreen_ = true;
    wasMaximized_ = isMaximized();
    applyLayoutVisibility();
    view_->setStyleSheet("QGraphicsView { border-radius: 0; }");
    showFullScreen();
    for (Overlay* ov : overlays_) { ov->setAcceptedMouseButtons(Qt::NoButton); ov->update(); }
    cursorTimer_->start();
    QTimer::singleShot(150, this, [this] { placeVideoControls(); showFullscreenHint(); });
}

void MainWindow::exitFullscreen() {
    fullscreen_ = false;
    cursorTimer_->stop();
    view_->viewport()->unsetCursor();
    view_->setStyleSheet(QString());
    wasMaximized_ ? showMaximized() : showNormal();
    applyLayoutVisibility();
    setEditMode(btnEdit_->isChecked());
    fsHint_->hide();
    QTimer::singleShot(150, this, [this] { placeVideoControls(); });
}

// ---------------------------------------------------------------- Datei
void MainWindow::dragEnterEvent(QDragEnterEvent* e) {
    if (e->mimeData()->hasUrls()) e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* e) {
    const auto urls = e->mimeData()->urls();
    if (!urls.isEmpty()) openFile(urls.first().toLocalFile());
}

void MainWindow::openDialog() {
    QString f = QFileDialog::getOpenFileName(this, T("open_video"), QString(),
                                             "Video (*.mp4 *.mov *.mkv *.m4v *.avi *.webm *.wmv *.flv);;*.*");
    if (!f.isEmpty()) openFile(f);
}

void MainWindow::openSettings() {
    SettingsDialog dlg(this);
    connect(&dlg, &SettingsDialog::languageChanged, this, [this](const QString& c) {
        setLanguage(c);
        QSettings().setValue("language", c);
        retranslate();
    });
    connect(&dlg, &SettingsDialog::themeChanged, this, [this](const QString& id) {
        setCurrentTheme(id);
        QSettings().setValue("theme", id);
        applyTheme();
    });
    dlg.exec();
}

void MainWindow::openFile(const QString& path) {
    player_->stop();
    for (auto& [id, ap] : audioPlayers_) { ap.p->stop(); ap.p->deleteLater(); ap.out->deleteLater(); }
    audioPlayers_.clear();
    pr_ = Project();
    Source s;
    s.path = path;
    pr_.sources.push_back(s);
    undo_.clear();
    redo_.clear();
    t_ = 0;
    cur_ = 0;
    selPiece_ = -1;
    selItem_ = 0;
    selAudio_ = 0;
    loading_ = true;
    canvasSet_ = false;
    loadedSrc_ = 0;
    pending_ = false;
    lblHint_->hide();
    setWindowTitle("Cutline – " + QFileInfo(path).fileName());
    player_->setSource(QUrl::fromLocalFile(path));
    player_->pause();  // erstes Bild sofort anzeigen
    rebuildOverlays();
    refreshInspector();
    timeline_->relayout();
}

void MainWindow::onDuration(qint64 ms) {
    if (ms > 0 && loading_) {
        loading_ = false;
        pr_.sources[0].duration = ms / 1000.0;
        pr_.pieces = {Piece{0, 0.0, ms / 1000.0, 1.0}};
        timeline_->relayout();
        updateUi();
        QTimer::singleShot(100, this, [this] {
            if (pr_.sources.isEmpty()) return;
            startThumbs(0);
        });
    }
}

void MainWindow::onNativeSize(const QSizeF& s) {
    if (!s.isValid() || s.isEmpty()) return;
    if (!canvasSet_) {
        canvasSet_ = true;
        canvasBg_->setVisible(true);
        pr_.vw = int(s.width());
        pr_.vh = int(s.height());
        scene_->setSceneRect(0, 0, pr_.vw, pr_.vh);
        canvasBg_->setRect(0, 0, pr_.vw, pr_.vh);
        rebuildOverlays();
    }
    fitVideoItem();
    fitView();
}

void MainWindow::onStatus(QMediaPlayer::MediaStatus st) {
    if (st == QMediaPlayer::LoadedMedia || st == QMediaPlayer::BufferedMedia) {
        onNativeSize(vitem_->nativeSize());
        if (pending_) {
            pending_ = false;
            player_->setPlaybackRate(pendingRate_);
            applyPlayerVolume();
            player_->setPosition(qint64(pendingPos_ * 1000));
            if (pendingPlay_) player_->play(); else player_->pause();
        }
    }
}

// ---------------------------------------------------------------- Bildfluss / Ambient
QImage MainWindow::frameImage() {
    if (!frameImgValid_) {
        frameImg_ = frame_.isValid() ? frame_.toImage() : QImage();
        frameImgValid_ = true;
    }
    return frameImg_;
}

void MainWindow::onFrame(const QVideoFrame& f) {
    frame_ = f;
    frameImgValid_ = false;
    bool blurVisible = false;
    for (Overlay* ov : overlays_) {
        Item* it = pr_.item(ov->itemId());
        if (it && it->kind == Item::Blur && ov->isVisible()) { ov->update(); blurVisible = true; }
    }
    (void)blurVisible;
    // Ambient-Licht nur, wenn die Ränder frei sind
    const QSize vp = view_->viewport()->size();
    const double viewAr = double(vp.width()) / std::max(1, vp.height());
    const double vidAr = double(pr_.vw) / std::max(1, pr_.vh);
    if (std::abs(viewAr - vidAr) > 0.02 && ambientClock_.elapsed() > 90) {
        ambientClock_.restart();
        QImage im = frameImage();
        if (!im.isNull()) view_->setAmbient(im.scaled(16, 9, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
}

// ---------------------------------------------------------------- Wiedergabe
void MainWindow::onState(QMediaPlayer::PlaybackState st) {
    if (st == QMediaPlayer::PlayingState) timer_->start();
    else timer_->stop();
    updatePlayIcon();
    updateAudio(true);
}

void MainWindow::togglePlay() {
    if (pr_.pieces.isEmpty()) return;
    if (player_->playbackState() == QMediaPlayer::PlayingState) {
        player_->pause();
    } else {
        if (t_ >= pr_.total() - 0.05) seek(0);
        player_->play();
    }
}

// Abschnitt aktivieren; bei anderer Quelldatei wird nachgeladen
void MainWindow::activate(int i, double srcPos, bool play) {
    cur_ = i;
    const Piece& p = pr_.pieces[i];
    if (p.src != loadedSrc_) {
        loadedSrc_ = p.src;
        pending_ = true;
        pendingPlay_ = play;
        pendingPos_ = srcPos;
        pendingRate_ = p.speed;
        player_->setSource(QUrl::fromLocalFile(pr_.sources[p.src].path));
        player_->pause();
        return;
    }
    if (pending_) {  // Ladevorgang läuft noch: Ziel aktualisieren
        pendingPos_ = srcPos;
        pendingRate_ = p.speed;
        pendingPlay_ = play;
        return;
    }
    player_->setPlaybackRate(p.speed);
    applyPlayerVolume();
    // Nur springen, wenn es wirklich woanders hingeht; bis der Player angekommen ist, liefert er noch die alte Position
    if (std::abs(player_->position() / 1000.0 - srcPos) > 0.04) {
        player_->setPosition(qint64(srcPos * 1000));
        seeking_ = true;
        seekSrc_ = srcPos;
        seekClock_.restart();
    }
    if (play && player_->playbackState() != QMediaPlayer::PlayingState) player_->play();
}

void MainWindow::applyPlayerVolume() {
    const double pv = cur_ >= 0 && cur_ < pr_.pieces.size() ? pr_.pieces[cur_].volume : 1.0;
    audio_->setVolume(float(std::clamp(pv * masterVol_, 0.0, 1.0)));
}

// Beim Ziehen: Zeit/Anzeige sofort, echte Sprünge höchstens alle 120 ms (sonst staut sich der Decoder)
void MainWindow::scrub(double t) {
    if (pr_.pieces.isEmpty()) return;
    scrubTarget_ = std::clamp(t, 0.0, pr_.total());
    const qint64 since = seekThrottle_.isValid() ? seekThrottle_.elapsed() : 1000;
    if (since >= 120) {
        seekTimer_->stop();
        seekThrottle_.restart();
        seek(scrubTarget_);
        return;
    }
    t_ = scrubTarget_;
    updateUi();
    if (!seekTimer_->isActive()) seekTimer_->start(int(120 - since));
}

// Vorschaubild + Zeit über dem Regler (sofort, ohne auf den Decoder zu warten)
void MainWindow::showScrubPreview(int v) {
    if (pr_.pieces.isEmpty()) return;
    const double t = v / 10000.0 * pr_.total();
    double src = 0;
    const int i = pr_.locate(t, &src);
    QImage thumb;
    if (const ThumbSet* ts = thumbs(pr_.pieces[i].src); ts && !ts->imgs.isEmpty())
        thumb = ts->imgs[std::clamp(int(src / ts->step), 0, int(ts->imgs.size()) - 1)]
                    .scaledToHeight(96, Qt::SmoothTransformation);
    const QString label = fmtTime(t);
    QFont f = font();
    f.setBold(true);
    const QFontMetrics fm(f);
    const int w = std::max(thumb.isNull() ? 0 : thumb.width() + 8, fm.horizontalAdvance(label) + 20);
    const int h = (thumb.isNull() ? 0 : thumb.height() + 4) + fm.height() + 12;
    QPixmap pm(w * devicePixelRatio(), h * devicePixelRatio());
    pm.setDevicePixelRatio(devicePixelRatio());
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QColor(255, 255, 255, 60));
    p.setBrush(QColor(12, 13, 18, 230));
    p.drawRoundedRect(QRectF(0.5, 0.5, w - 1, h - 1), 10, 10);
    if (!thumb.isNull()) p.drawImage(QPoint((w - thumb.width()) / 2, 4), thumb);
    p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(QRect(0, h - fm.height() - 8, w, fm.height() + 4), Qt::AlignCenter, label);
    p.end();
    scrubPrev_->setPixmap(pm);
    scrubPrev_->resize(w, h);
    const QPoint base = slider_->mapTo(this, QPoint(0, 0));
    const int hx = base.x() + 7 + int(double(v) / slider_->maximum() * (slider_->width() - 14));
    scrubPrev_->move(std::clamp(hx - w / 2, 4, width() - w - 4), base.y() - h - 8);
    scrubPrev_->show();
    scrubPrev_->raise();
}

void MainWindow::seek(double t) {
    if (pr_.pieces.isEmpty()) return;
    seekTimer_->stop();  // ein direkter Sprung ersetzt einen noch ausstehenden gedrosselten
    t = std::clamp(t, 0.0, pr_.total());
    double src = 0;
    int i = pr_.locate(t, &src);
    activate(i, src, player_->playbackState() == QMediaPlayer::PlayingState);
    t_ = t;
    updateUi();
    updateAudio(true);
}

// Abschnittswechsel während der Wiedergabe: springt zum nächsten Abschnitt und setzt das Tempo
void MainWindow::tick() {
    if (pr_.pieces.isEmpty() || pending_) return;
    cur_ = std::min<int>(cur_, pr_.pieces.size() - 1);
    const Piece* p = &pr_.pieces[cur_];
    double src = player_->position() / 1000.0;
    if (seeking_) {  // Sprung noch unterwegs: alte Position nicht anzeigen
        if (std::abs(src - seekSrc_) < 0.75 || seekClock_.elapsed() > 2000) seeking_ = false;
        else return;
    }
    bool ended = player_->mediaStatus() == QMediaPlayer::EndOfMedia;
    if (src >= p->end - 0.03 || ended) {
        if (cur_ + 1 < pr_.pieces.size()) {
            activate(cur_ + 1, pr_.pieces[cur_ + 1].start, true);
            t_ = pr_.outStart(cur_);
            updateUi();
            updateAudio(false);
            return;
        }
        player_->pause();
        t_ = pr_.total();
        updateUi();
        return;
    }
    t_ = pr_.outStart(cur_) + std::max(0.0, src - p->start) / p->speed;
    updateUi();
    updateAudio(false);
}

void MainWindow::updateUi() {
    double total = pr_.total();
    lblTime_->setText(fmtTime(t_) + " / " + fmtTime(total));
    if (!slider_->isSliderDown()) {
        QSignalBlocker b(slider_);
        slider_->setValue(total > 0 ? int(t_ / total * 10000) : 0);
    }
    for (Overlay* ov : overlays_) {
        if (Item* it = pr_.item(ov->itemId())) ov->setVisible(it->t0 <= t_ && t_ < it->t1);
    }
    timeline_->update();
}

// ---------------------------------------------------------------- Ton-Spuren (Vorschau)
void MainWindow::syncAudioPlayers() {
    for (auto it = audioPlayers_.begin(); it != audioPlayers_.end();) {
        if (!pr_.audio(it->first)) {
            it->second.p->stop();
            it->second.p->deleteLater();
            it->second.out->deleteLater();
            it = audioPlayers_.erase(it);
        } else {
            ++it;
        }
    }
    for (const AudioClip& a : pr_.audios) {
        if (audioPlayers_.count(a.id)) continue;
        AudioPlayer ap{new QMediaPlayer(this), new QAudioOutput(this)};
        ap.p->setAudioOutput(ap.out);
        ap.p->setSource(QUrl::fromLocalFile(a.path));
        audioPlayers_[a.id] = ap;
    }
}

void MainWindow::updateAudio(bool force) {
    const bool playing = player_->playbackState() == QMediaPlayer::PlayingState;
    for (auto& [id, ap] : audioPlayers_) {
        AudioClip* a = pr_.audio(id);
        if (!a) continue;
        ap.out->setVolume(float(std::clamp(a->volume * masterVol_, 0.0, 1.0)));
        const bool inRange = t_ >= a->t0 && t_ < a->t0 + a->dur;
        if (playing && inRange) {
            qint64 want = qint64((a->srcStart + (t_ - a->t0)) * 1000);
            if (ap.p->playbackState() != QMediaPlayer::PlayingState) {
                ap.p->setPosition(want);
                ap.p->play();
            } else if (force || std::llabs(ap.p->position() - want) > 300) {
                ap.p->setPosition(want);
            }
        } else if (ap.p->playbackState() == QMediaPlayer::PlayingState) {
            ap.p->pause();
        }
    }
}

// ---------------------------------------------------------------- Vorschaubilder
const ThumbSet* MainWindow::thumbs(int srcIndex) const {
    if (srcIndex < 0 || srcIndex >= pr_.sources.size()) return nullptr;
    auto it = thumbSets_.find(pr_.sources[srcIndex].path);
    return it == thumbSets_.end() ? nullptr : it->second.get();
}

void MainWindow::startThumbs(int si) {
    if (si < 0 || si >= pr_.sources.size()) return;
    const Source& s = pr_.sources[si];
    if (thumbSets_.count(s.path) || s.duration <= 0) return;
    auto ts = std::make_shared<ThumbSet>();
    ts->step = std::max(0.5, s.duration / 110.0);
    thumbSets_[s.path] = ts;
    auto dir = std::make_unique<QTemporaryDir>();
    const QString d = dir->path();
    thumbDirs_.push_back(std::move(dir));
    auto* p = new QProcess(this);
    connect(p, &QProcess::finished, this, [this, p, d, ts] {
        const QStringList files = QDir(d).entryList({"t*.jpg"}, QDir::Files, QDir::Name);
        for (const QString& f : files) {
            QImage im(QDir(d).filePath(f));
            if (!im.isNull()) ts->imgs.append(im);
        }
        p->deleteLater();
        timeline_->update();
    });
    p->start(ffmpegPath(), {"-hide_banner", "-v", "error", "-skip_frame", "nokey", "-i", s.path, "-an", "-sn",
                            "-vf", QString("fps=1/%1,scale=-2:80").arg(ts->step, 0, 'f', 3), "-q:v", "5",
                            d + "/t%05d.jpg"});
}

void MainWindow::ensureProbed() {
    for (Source& s : pr_.sources) {
        if (s.probed) continue;
        MediaInfo m = probeMedia(s.path);
        s.fps = m.fps;
        s.hasAudio = m.hasAudio;
        if (s.duration <= 0) s.duration = m.duration;
        s.probed = true;
    }
}

// ---------------------------------------------------------------- Bearbeiten
void MainWindow::pushUndo() {
    undo_.push_back(pr_.snapshot());
    if (undo_.size() > 100) undo_.erase(undo_.begin());
    redo_.clear();
}

void MainWindow::doUndo() {
    if (undo_.empty()) return;
    redo_.push_back(pr_.snapshot());
    pr_.restore(undo_.back());
    undo_.pop_back();
    afterRestore();
}

void MainWindow::doRedo() {
    if (redo_.empty()) return;
    undo_.push_back(pr_.snapshot());
    pr_.restore(redo_.back());
    redo_.pop_back();
    afterRestore();
}

void MainWindow::afterRestore() {
    selItem_ = 0;
    selAudio_ = 0;
    selPiece_ = std::min<int>(selPiece_, pr_.pieces.size() - 1);
    modelEdited();
}

void MainWindow::modelEdited(bool keepTime) {
    t_ = std::clamp(t_, 0.0, pr_.total());
    if (!keepTime) {
        rebuildOverlays();
        syncAudioPlayers();
    }
    timeline_->relayout();
    refreshInspector();
    if (!keepTime) seek(t_);
    updateUi();
    updateAudio(false);
}

void MainWindow::rebuildOverlays() {
    for (Overlay* ov : overlays_) {
        scene_->removeItem(ov);
        delete ov;
    }
    overlays_.clear();
    for (const Item& it : pr_.items) {
        auto* ov = new Overlay(it.id, this);
        ov->setAcceptedMouseButtons(showGuides() ? Qt::LeftButton : Qt::NoButton);
        scene_->addItem(ov);
        overlays_.push_back(ov);
    }
    updateUi();
}

void MainWindow::selectPiece(int i) {
    selPiece_ = i;
    if (i >= 0) { selItem_ = 0; selAudio_ = 0; }
    refreshInspector();
    timeline_->update();
    for (Overlay* ov : overlays_) ov->update();
}

void MainWindow::selectItem(int id) {
    selItem_ = id;
    if (id) { selPiece_ = -1; selAudio_ = 0; }
    refreshInspector();
    timeline_->update();
    for (Overlay* ov : overlays_) ov->update();
}

void MainWindow::selectAudio(int id) {
    selAudio_ = id;
    if (id) { selPiece_ = -1; selItem_ = 0; }
    refreshInspector();
    timeline_->update();
    for (Overlay* ov : overlays_) ov->update();
}

void MainWindow::refreshInspector() {
    Item* it = selItem_ ? pr_.item(selItem_) : nullptr;
    AudioClip* au = selAudio_ ? pr_.audio(selAudio_) : nullptr;
    if (au) {
        insp_->setCurrentIndex(2);
        lblItem_->setText(T("audio_el"));
        { QSignalBlocker b(spT0_); spT0_->setValue(au->t0); }
        { QSignalBlocker b(spT1_); spT1_->setValue(au->t0 + au->dur); }
        lblStr_->setText(T("volume") + QString("  %1 %").arg(qRound(au->volume * 100)));
        QSignalBlocker b(sliderStr_);
        sliderStr_->setRange(0, 200);
        sliderStr_->setValue(int(au->volume * 100));
    } else if (it) {
        insp_->setCurrentIndex(2);
        lblItem_->setText(it->kind == Item::Image ? T("image") : T("blur_area"));
        { QSignalBlocker b(spT0_); spT0_->setValue(it->t0); }
        { QSignalBlocker b(spT1_); spT1_->setValue(it->t1); }
        lblStr_->setText(it->kind == Item::Image ? T("size") : T("strength"));
        QSignalBlocker b(sliderStr_);
        sliderStr_->setRange(5, 100);
        sliderStr_->setValue(it->kind == Item::Image ? int(it->w * 100) : int(it->strength));
    } else if (selPiece_ >= 0 && selPiece_ < pr_.pieces.size()) {
        const Piece& p = pr_.pieces[selPiece_];
        insp_->setCurrentIndex(1);
        { QSignalBlocker b(spSpeed_); spSpeed_->setValue(p.speed); }
        { QSignalBlocker b(sliderPieceVol_); sliderPieceVol_->setValue(qRound(p.volume * 100)); }
        lblPieceVol_->setText(T("volume") + QString("  %1 %").arg(qRound(p.volume * 100)));
        lblPiece_->setText(T("source_range").arg(fmtTime(p.start), fmtTime(p.end)) + "\n" +
                           T("result_dur").arg(fmtTime(p.outDur())));
    } else {
        insp_->setCurrentIndex(0);
    }
}

void MainWindow::setSpeed(double s) {
    if (selPiece_ >= 0 && selPiece_ < pr_.pieces.size() && pr_.pieces[selPiece_].speed != s) {
        pushUndo();
        pr_.pieces[selPiece_].speed = s;
        modelEdited();
    }
}

void MainWindow::setPieceVolume(int v) {
    if (selPiece_ < 0 || selPiece_ >= pr_.pieces.size()) return;
    pr_.pieces[selPiece_].volume = v / 100.0;
    lblPieceVol_->setText(T("volume") + QString("  %1 %").arg(v));
    applyPlayerVolume();
    timeline_->update();
}

void MainWindow::itemTimesChanged() {
    double total = pr_.total();
    if (AudioClip* a = selAudio_ ? pr_.audio(selAudio_) : nullptr) {
        pushUndo();
        a->t0 = std::max(0.0, spT0_->value());
        a->dur = std::clamp(spT1_->value() - a->t0, 0.1, a->fileDur - a->srcStart);
        modelEdited(true);
        refreshInspector();
        return;
    }
    Item* it = selItem_ ? pr_.item(selItem_) : nullptr;
    if (!it) return;
    pushUndo();
    it->t0 = std::min(spT0_->value(), total);
    it->t1 = std::max(it->t0 + 0.1, std::min(spT1_->value(), total));
    modelEdited(true);
}

void MainWindow::sliderChanged(int v) {
    if (AudioClip* a = selAudio_ ? pr_.audio(selAudio_) : nullptr) {
        a->volume = v / 100.0;
        lblStr_->setText(T("volume") + QString("  %1 %").arg(v));
        updateAudio(false);
        return;
    }
    Item* it = selItem_ ? pr_.item(selItem_) : nullptr;
    if (!it) return;
    if (it->kind == Item::Blur) {
        it->strength = v;
        for (Overlay* ov : overlays_) ov->update();
    } else {
        it->w = v / 100.0;
        it->h = it->w * pr_.vw * it->ar / pr_.vh;
        it->x = std::min(it->x, std::max(0.0, 1 - it->w));
        it->y = std::min(it->y, std::max(0.0, 1 - it->h));
        rebuildOverlays();
    }
}

void MainWindow::deleteSelected() {
    if (selAudio_) {
        for (int i = 0; i < pr_.audios.size(); ++i)
            if (pr_.audios[i].id == selAudio_) {
                pushUndo();
                pr_.audios.removeAt(i);
                selAudio_ = 0;
                modelEdited();
                return;
            }
    }
    for (int i = 0; i < pr_.items.size(); ++i) {
        if (pr_.items[i].id == selItem_) {
            pushUndo();
            pr_.items.removeAt(i);
            selItem_ = 0;
            modelEdited();
            return;
        }
    }
}

void MainWindow::doSplit() {
    if (pr_.pieces.isEmpty()) return;
    double src = 0;
    int i = pr_.locate(t_, &src);
    Piece p = pr_.pieces[i];
    if (src - p.start < kMinPiece || p.end - src < kMinPiece) return;
    pushUndo();
    pr_.pieces[i].end = src;
    Piece rest = p;
    rest.start = src;
    pr_.pieces.insert(i + 1, rest);
    selPiece_ = i + 1;
    selItem_ = selAudio_ = 0;
    modelEdited(true);
}

void MainWindow::doDelete() {
    if (selItem_ || selAudio_) return deleteSelected();
    if (pr_.pieces.size() < 2 || selPiece_ < 0 || selPiece_ >= pr_.pieces.size()) return;
    pushUndo();
    pr_.pieces.removeAt(selPiece_);
    selPiece_ = std::min<int>(selPiece_, pr_.pieces.size() - 1);
    modelEdited();
}

void MainWindow::doTrimStart() {
    if (pr_.pieces.isEmpty()) return;
    double src = 0;
    int i = pr_.locate(t_, &src);
    if (pr_.pieces[i].end - src >= kMinPiece) {
        pushUndo();
        pr_.pieces[i].start = src;
        selPiece_ = i;
        modelEdited();
    }
}

void MainWindow::doTrimEnd() {
    if (pr_.pieces.isEmpty()) return;
    double src = 0;
    int i = pr_.locate(t_, &src);
    if (src - pr_.pieces[i].start >= kMinPiece) {
        pushUndo();
        pr_.pieces[i].end = src;
        selPiece_ = i;
        modelEdited();
    }
}

void MainWindow::newItem(Item it) {
    double total = pr_.total();
    double t0 = t_, t1 = std::min(total, t0 + 3);
    if (t1 - t0 < 0.5) { t0 = std::max(0.0, t1 - 3); t1 = total; }
    it.t0 = t0;
    it.t1 = t1;
    it.id = pr_.nextId++;
    pushUndo();
    pr_.items.append(it);
    selItem_ = it.id;
    selPiece_ = -1;
    selAudio_ = 0;
    modelEdited();
}

void MainWindow::addBlur() {
    if (pr_.pieces.isEmpty()) return;
    Item it;
    it.kind = Item::Blur;
    it.x = 0.35; it.y = 0.35; it.w = 0.3; it.h = 0.2;
    newItem(it);
}

void MainWindow::addImage() {
    if (pr_.pieces.isEmpty()) return;
    QString f = QFileDialog::getOpenFileName(this, T("choose_image"), QString(),
                                             "Images (*.png *.jpg *.jpeg *.webp *.bmp *.gif)");
    if (f.isEmpty()) return;
    QPixmap pm(f);
    if (pm.isNull()) return;
    Item it;
    it.kind = Item::Image;
    it.path = f;
    it.ar = double(pm.height()) / pm.width();
    it.x = 0.05; it.y = 0.05; it.w = 0.25;
    it.h = it.w * pr_.vw * it.ar / pr_.vh;
    newItem(it);
}

// Ein Knopf für Video und Ton: die Art wird an der Datei erkannt
void MainWindow::addMedia() {
    QString f = QFileDialog::getOpenFileName(
        this, T("add_media"), QString(),
        T("media_files") + " (*.mp4 *.mov *.mkv *.m4v *.avi *.webm *.wmv *.flv *.ts "
                           "*.mp3 *.wav *.m4a *.aac *.flac *.ogg *.opus *.wma);;*.*");
    if (f.isEmpty()) return;
    if (pr_.pieces.isEmpty()) return openFile(f);
    MediaInfo mi = probeMedia(f);
    if (!mi.ok || mi.duration <= 0) {
        QMessageBox::warning(this, "Cutline", T("media_unreadable"));
        return;
    }
    if (mi.w > 0 && mi.h > 0) addVideoFile(f, mi);
    else addAudioFile(f, mi);
}

// Weiteres Video an der Playhead-Position einfügen
void MainWindow::addVideoFile(const QString& f, const MediaInfo& mi) {
    pushUndo();
    Source s;
    s.path = f; s.duration = mi.duration; s.fps = mi.fps; s.hasAudio = mi.hasAudio; s.probed = true;
    pr_.sources.push_back(s);
    const int si = pr_.sources.size() - 1;

    double src = 0;
    int i = pr_.locate(t_, &src);
    const Piece p = pr_.pieces[i];
    int at;
    if (src - p.start < kMinPiece) {
        at = i;
    } else if (p.end - src < kMinPiece) {
        at = i + 1;
    } else {
        pr_.pieces[i].end = src;
        Piece rest = p;
        rest.start = src;
        pr_.pieces.insert(i + 1, rest);
        at = i + 1;
    }
    pr_.pieces.insert(at, Piece{si, 0.0, mi.duration, 1.0, 1.0});
    selPiece_ = at;
    selItem_ = selAudio_ = 0;
    t_ = pr_.outStart(at);
    startThumbs(si);
    modelEdited();
}

void MainWindow::addAudioFile(const QString& f, const MediaInfo& mi) {
    AudioClip a;
    a.id = pr_.nextId++;
    a.path = f;
    a.t0 = t_;
    a.fileDur = mi.duration;
    a.dur = std::min(mi.duration, std::max(0.5, pr_.total() - t_));
    pushUndo();
    pr_.audios.append(a);
    selAudio_ = a.id;
    selPiece_ = -1;
    selItem_ = 0;
    modelEdited();
}

// ---------------------------------------------------------------- Export
void MainWindow::exportVideo() {
    if (pr_.pieces.isEmpty() || exportProc_) return;
    player_->pause();
    ensureProbed();
    ExportDialog opt(this, pr_.vw, pr_.vh, pr_.sources[0].fps, pr_.sources[0].path);
    if (opt.exec() != QDialog::Accepted) return;
    ExportOptions eo = opt.options();
    const bool replace = opt.replaceOriginal();
    const QString original = pr_.sources[0].path;
    const QString finalPath = opt.outputPath();
    // Beim Ersetzen erst in eine Zwischendatei schreiben (ffmpeg liest das Original noch)
    const QString out = replace ? QFileInfo(finalPath).dir().filePath(".cutline-export-" +
                                  QString::number(QDateTime::currentMSecsSinceEpoch()) + ".mp4")
                                : finalPath;

    QStringList args = buildExport(pr_, eo, out);
    const double total = pr_.total();
    auto* dlg = new QProgressDialog(T("exporting"), T("cancel"), 0, 1000, this);
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setMinimumDuration(0);
    dlg->setAutoClose(false);
    dlg->setAutoReset(false);
    dlg->setValue(0);

    exportProc_ = new QProcess(this);
    auto* proc = exportProc_;
    auto errText = std::make_shared<QString>();
    connect(proc, &QProcess::readyReadStandardOutput, this, [proc, dlg, total] {
        while (proc->canReadLine()) {
            QString line = QString::fromUtf8(proc->readLine()).trimmed();
            if (line.startsWith("out_time_us=")) {
                double s = line.mid(12).toLongLong() / 1e6;
                dlg->setValue(int(std::min(1.0, s / std::max(total, 0.01)) * 1000));
            }
        }
    });
    connect(proc, &QProcess::readyReadStandardError, this,
            [proc, errText] { *errText += QString::fromUtf8(proc->readAllStandardError()); });
    connect(dlg, &QProgressDialog::canceled, proc, [proc] { proc->kill(); });
    connect(proc, &QProcess::finished, this, [this, proc, dlg, out, errText, replace, original, finalPath](int code, QProcess::ExitStatus st) {
        bool cancelled = st == QProcess::CrashExit;
        dlg->close();
        dlg->deleteLater();
        exportProc_ = nullptr;
        proc->deleteLater();
        if (cancelled || code != 0) {
            if (replace) QFile::remove(out);
            if (!cancelled) QMessageBox::critical(this, T("export_fail"), errText->right(1500));
            return;
        }
        QString saved = out;
        if (replace) {
            // Datei freigeben, Original ersetzen, Ergebnis öffnen
            player_->stop();
            player_->setSource(QUrl());
            for (auto& [id, ap] : audioPlayers_) ap.p->setSource(QUrl());
            QApplication::processEvents();
            thumbSets_.erase(original);
            thumbSets_.erase(finalPath);
            bool ok = true;
            if (QFileInfo::exists(finalPath) && !QFile::remove(finalPath)) ok = false;
            if (ok && original != finalPath) QFile::remove(original);
            if (ok) ok = QFile::rename(out, finalPath);
            if (!ok) {
                QMessageBox::critical(this, T("export_fail"), QDir::toNativeSeparators(out));
                return;
            }
            saved = finalPath;
            openFile(finalPath);
        }
        QMessageBox::information(this, T("done"), T("saved") + "\n" + QDir::toNativeSeparators(saved));
    });
    proc->start(ffmpegPath(), args);
}

// Entwickler-Test: Fenster per grab() in Dateien rendern (greift nicht auf den Bildschirm zu)
void MainWindow::selfShots(const QString& dir) {
    auto shot = [this, dir](const QString& name) { grab().save(dir + "/" + name + ".png"); };
    QTimer::singleShot(3500, this, [=] {
        shot("a_default");
        btnEdit_->setChecked(true);
        seek(2.0);
        doSplit();
        addBlur();
        seek(2.5);
        selectPiece(0);
        sliderPieceVol_->setValue(50);
        // Spulen: viele Ziehschritte hintereinander dürfen den Player nicht fluten
        for (int k = 0; k < 40; ++k) scrub(pr_.total() * k / 40.0);
        seek(1.0);
        showScrubPreview(4200);
    });
    QTimer::singleShot(5500, this, [=] {
        shot("b_edit");
        scrubPrev_->hide();
        enterFullscreen();
    });
    QTimer::singleShot(7500, this, [=] {
        shot("c_fullscreen");
        exitFullscreen();
        setCurrentTheme("xp");
        applyTheme();
    });
    QTimer::singleShot(9500, this, [=] {
        shot("d_xp");
        setCurrentTheme("light");
        applyTheme();
        setLanguage("ja");
        retranslate();
    });
    QTimer::singleShot(11500, this, [=] {
        shot("e_light_ja");
        setCurrentTheme("violet");
        setLanguage("de");
        applyTheme();
        retranslate();
        SettingsDialog d(this);
        QTimer::singleShot(300, &d, [&d, dir] { d.grab().save(dir + "/f_settings.png"); d.accept(); });
        d.exec();
        ExportDialog e(this, pr_.vw, pr_.vh, 30.0, pr_.sources.isEmpty() ? QString() : pr_.sources[0].path);
        QTimer::singleShot(300, &e, [&e, dir] { e.grab().save(dir + "/g_export.png"); e.accept(); });
        e.exec();
    });
    // Alle Themes einzeln (für die Website)
    QTimer::singleShot(13000, this, [=] {
        setLanguage("en");
        retranslate();
        const auto list = themes();
        for (int i = 0; i < list.size(); ++i) {
            QTimer::singleShot(i * 900, this, [=] {
                setCurrentTheme(list[i].id);
                applyTheme();
                QTimer::singleShot(400, this, [=] { shot("theme_" + list[i].id); });
            });
        }
        QTimer::singleShot(list.size() * 900 + 800, this, [] { QApplication::quit(); });
    });
}
