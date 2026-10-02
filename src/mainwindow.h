#pragma once
#include <QAudioOutput>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QLabel>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QStackedWidget>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QTemporaryDir>
#include <QTimer>
#include <QVideoFrame>
#include <QMouseEvent>
#include <QStyleOptionSlider>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include "host.h"
#include "icons.h"
#include "overlay.h"
#include "timeline.h"
#include "updater.h"
#include "videoview.h"

// Zeitleiste unten: Klick springt direkt an die Stelle (statt in kleinen Schritten)
class SeekSlider : public QSlider {
public:
    SeekSlider() : QSlider(Qt::Horizontal) {}

protected:
    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            QStyleOptionSlider opt;
            initStyleOption(&opt);
            const QRect handle = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
            if (!handle.contains(e->position().toPoint())) {
                const int v = QStyle::sliderValueFromPosition(minimum(), maximum(),
                                                              int(e->position().x()) - handle.width() / 2,
                                                              width() - handle.width());
                setValue(v);
                emit sliderMoved(v);
            }
        }
        QSlider::mousePressEvent(e);
    }
};

class Osd;

class MainWindow : public QMainWindow, public EditorHost {
    Q_OBJECT
public:
    MainWindow();
    void openFile(const QString& path);
    void selfShots(const QString& dir);
    void aspectShots(const QString& dir);  // Entwickler-Test: Bildformat umstellen, rendern, exportieren

    // EditorHost
    Project& pr() override { return pr_; }
    double curTime() const override { return t_; }
    int selPiece() const override { return selPiece_; }
    int selItem() const override { return selItem_; }
    int selAudio() const override { return selAudio_; }
    void selectPiece(int i) override;
    void selectItem(int id) override;
    void selectAudio(int id) override;
    void pushUndo() override;
    void modelEdited(bool keepTime = false) override;
    void seek(double t) override;
    void scrub(double t) override;
    double viewScale() const override { return view_->transform().m11(); }
    int timelineViewportWidth() const override { return tlScroll_->viewport()->width(); }
    const ThumbSet* thumbs(int srcIndex) const override;
    const WaveSet* waves(const QString& path) const override;
    void timelineZoomed() override;
    QImage frameImage() override;
    QRectF videoRect() const override { return QRectF(vitem_->pos(), vitem_->size()); }
    bool showGuides() const override { return !fullscreen_ && btnEdit_->isChecked(); }
    void insertMediaAt(const QString& path, double t) override;

protected:
    void resizeEvent(QResizeEvent*) override;
    bool eventFilter(QObject*, QEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;
    void dropEvent(QDropEvent*) override;
    void closeEvent(QCloseEvent*) override;

private:
    struct BtnSpec {
        QAbstractButton* b;
        const char* text;  // Übersetzungsschlüssel, nullptr = nur Icon
        const char* tip;
        Ic icon;
        const char* tipSuffix;
        bool onAccent;
    };
    struct AudioPlayer {
        QMediaPlayer* p;
        QAudioOutput* out;
    };

    QPushButton* mk(const char* textKey, const char* tipKey, Ic ic, const char* objName = nullptr,
                    const char* tipSuffix = "");
    void buildUi();
    QWidget* buildInspector();
    void addShortcut(const QString& id, std::function<void()> fn);
    void applyShortcuts();      // Tastenkürzel aus den Einstellungen übernehmen (auch Tooltips)
    void toggleFsBar();         // Leiste im Vollbild ein-/ausblenden
    void updateFsBar();
    void applyTheme();
    void retranslate();
    void updatePlayIcon();
    void setEditMode(bool on);
    void applyLayoutVisibility();
    void toggleFullscreen();
    void placeVideoControls();
    void showFullscreenHint();
    void enterFullscreen();
    void exitFullscreen();
    void openDialog();
    void openSettings();
    void fitView();
    void fitVideoItem();
    void applyCanvas();
    void setAspect(int aspect);

    void onDuration(qint64 ms);
    void onNativeSize(const QSizeF& s);
    void onState(QMediaPlayer::PlaybackState st);
    void onStatus(QMediaPlayer::MediaStatus st);
    void onFrame(const QVideoFrame& f);
    void activate(int piece, double srcPos, bool play);
    void togglePlay();
    void tick();
    void updateUi();
    void syncAudioPlayers();
    void updateAudio(bool force);
    void startThumbs(int srcIndex);
    void ensureProbed();

    void doUndo();
    void doRedo();
    void afterRestore();
    void rebuildOverlays();
    void refreshInspector();
    void setSpeed(double s);
    void itemTimesChanged();
    void sliderChanged(int v);
    void deleteSelected();
    void doSplit();
    void doDelete();
    void doTrimStart();
    void doTrimEnd();
    void newItem(Item it);
    void addBlur();
    void addImage();
    // Medien-Sammlung (rechts) und Einfügen
    QWidget* buildMediaPage();
    void importMedia();
    void addToBin(const QString& path);
    void insertFromBin(const QString& path);
    void addImageFile(const QString& path);
    void addText();
    // Bild anpassen (Größe/Position/Drehung) und Wellenformen
    void editPieceTf(const std::function<void(Piece&)>& fn);
    void applyVideoTransform();
    void startWaves(const QString& path);
    void pickItemColor(bool background);
    void addVideoFile(const QString& f, const MediaInfo& mi);
    void addAudioFile(const QString& f, const MediaInfo& mi);
    void setPieceVolume(int v);
    void applyPlayerVolume();
    void showScrubPreview(int sliderValue);
    void exportVideo();

    Project pr_;
    double t_ = 0;
    int cur_ = 0;
    int selPiece_ = -1, selItem_ = 0, selAudio_ = 0;
    std::vector<Snapshot> undo_, redo_;
    std::vector<Overlay*> overlays_;
    bool loading_ = false, canvasSet_ = false, fullscreen_ = false, wasMaximized_ = false;

    // Player-Zustand
    int loadedSrc_ = -1;
    bool pending_ = false, pendingPlay_ = false;
    double pendingPos_ = 0, pendingRate_ = 1;
    double masterVol_ = 1.0;  // Lautstärke wird pro Abschnitt / Ton-Clip eingestellt
    // Spulen: Player-Sprünge drosseln, bis der Sprung angekommen ist, die alte Position ignorieren
    QTimer* seekTimer_ = nullptr;
    QElapsedTimer seekThrottle_, seekClock_;
    double scrubTarget_ = 0, seekSrc_ = 0;
    bool seeking_ = false;
    std::map<int, AudioPlayer> audioPlayers_;

    // Bild
    QMediaPlayer* player_;
    QAudioOutput* audio_;
    QGraphicsScene* scene_;
    QGraphicsRectItem* canvasBg_;
    QGraphicsVideoItem* vitem_;
    VideoView* view_;
    QTimer *timer_, *cursorTimer_;
    QVideoFrame frame_;
    QImage frameImg_;
    bool frameImgValid_ = false;
    QElapsedTimer ambientClock_;

    // Vorschaubilder
    std::map<QString, std::shared_ptr<ThumbSet>> thumbSets_;
    std::vector<std::unique_ptr<QTemporaryDir>> thumbDirs_;

    // UI
    std::vector<BtnSpec> btns_;
    QPushButton *btnPlay_, *btnEdit_, *fsBtn_ = nullptr;
    std::map<QString, QAction*> scActions_;  // Tastenkürzel nach Kennung (shortcuts.h)
    // Vollbild: kleine Leiste über dem Bild (Pause, Fortschritt, Zeit, Ausblenden)
    QFrame* fsBar_ = nullptr;
    QPushButton *fsPlay_ = nullptr, *fsHide_ = nullptr, *fsShow_ = nullptr;
    QSlider* fsSlider_ = nullptr;
    QLabel* fsTime_ = nullptr;
    bool fsBarVisible_ = true;
    Osd* osd_ = nullptr;  // Symbol in der Bildmitte bei Pause/Abspielen/Spulen (wie bei YouTube)
    QPushButton *tabMedia_, *tabProps_;
    QListWidget* bin_;
    QLabel* lblBinHint_;
    std::unique_ptr<QTemporaryDir> binDir_;
    QString lastSel_;
    // Bild anpassen
    QLabel *lblSize_, *lblPos_, *lblRot_;
    QSlider* sliderScale_;
    QDoubleSpinBox *spScale_, *spPosX_, *spPosY_, *spRot_;
    QPushButton* btnResetTf_;
    // Text
    QWidget* textBox_;
    QPlainTextEdit* textEdit_;
    QComboBox* fontBox_;
    QComboBox* aspectBox_ = nullptr;
    QLabel* lblAspect_ = nullptr;
    QCheckBox* chkBg_;
    QPushButton *btnColor_, *btnBgColor_;
    QLabel *lblFont_, *lblColors_;
    bool textUndo_ = false;
    std::map<QString, std::shared_ptr<WaveSet>> waves_;
    QLabel* fsHint_ = nullptr;
    QLabel* scrubPrev_ = nullptr;  // Vorschaubild über der Zeitleiste beim Ziehen
    QSlider *slider_, *sliderStr_;
    QLabel *lblTime_, *lblHint_, *lblPiece_, *lblItem_, *lblStr_;
    QLabel *lblInspHint_, *lblSegment_, *lblSpeed_, *lblStart_, *lblEnd_, *lblPieceVol_;
    QSlider *sliderPieceVol_, *sliderSpeed_;
    QCheckBox *chkPieceDenoise_, *chkAudioDenoise_;
    QPushButton* btnDelEl_;
    QWidget *topbar_, *transport_, *tools_;
    QStackedWidget* insp_;
    QFrame* inspCard_;
    QFrame* mediaCard_ = nullptr;
    double lastSeekT_ = -1;  // zuletzt angesprungene Stelle (Ausgabezeit)
    QDoubleSpinBox *spSpeed_, *spT0_, *spT1_;
    Timeline* timeline_;
    QScrollArea* tlScroll_;
    QVBoxLayout* rootLayout_;
    QProcess* exportProc_ = nullptr;
    Updater* updater_ = nullptr;
};
