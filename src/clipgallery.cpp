#include "clipgallery.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QVBoxLayout>
#include "i18n.h"
#include "icons.h"
#include "model.h"
#include "theme.h"

QString cliplineClipsDir() {
    const QString d = QSettings("WSoftware", "Clipline").value("clipsDir").toString();
    return !d.isEmpty() && QDir(d).exists() ? d : QString();
}

QString cliplineExe() {
    const QString p = QSettings("WSoftware", "Clipline").value("exePath").toString();
    return !p.isEmpty() && QFileInfo::exists(p) ? p : QString();
}

// Platzhalter-Kachel (16:9) mit Dauer-Plakette
static QIcon tile(const QImage& img, double dur) {
    const Theme& th = currentTheme();
    QPixmap pm(320, 180);
    pm.fill(th.panel2);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    if (!img.isNull()) {
        const QSize s = img.size().scaled(pm.size(), Qt::KeepAspectRatioByExpanding);
        p.drawImage(QRect((pm.width() - s.width()) / 2, (pm.height() - s.height()) / 2, s.width(), s.height()), img);
    } else {
        p.drawPixmap(QRect(130, 60, 60, 60), makeCliplinePixmap(60));
    }
    if (dur > 0) {
        const int d = int(dur + 0.5);
        const QString t = QString("%1:%2").arg(d / 60).arg(d % 60, 2, 10, QChar('0'));
        QFont f;
        f.setPixelSize(13);
        f.setBold(true);
        p.setFont(f);
        const QRectF b(pm.width() - 58, pm.height() - 30, 50, 22);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 170));
        p.drawRoundedRect(b, 11, 11);
        p.setPen(Qt::white);
        p.drawText(b, Qt::AlignCenter, t);
    }
    return QIcon(pm);
}

ClipGalleryDialog::ClipGalleryDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(T("clips") + " · Clipline");
    setWindowIcon(QIcon(makeCliplinePixmap(64)));
    resize(980, 640);
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(20, 18, 20, 18);
    v->setSpacing(12);

    auto* head = new QHBoxLayout;
    auto* logo = new QLabel;
    logo->setPixmap(makeCliplinePixmap(36));
    head->addWidget(logo);
    auto* title = new QLabel(T("clips"));
    title->setObjectName("title");
    title->setStyleSheet("font-size:20px;font-weight:700;");
    head->addWidget(title);
    const QString dir = cliplineClipsDir();
    auto* path = new QLabel(QDir::toNativeSeparators(dir));
    path->setObjectName("hint");
    head->addWidget(path);
    head->addStretch();
    if (!cliplineExe().isEmpty()) {
        auto* openCl = new QPushButton(QIcon(makeCliplinePixmap(64)), " " + T("open_clipline"));
        connect(openCl, &QPushButton::clicked, this, [] { QProcess::startDetached(cliplineExe(), {}); });
        head->addWidget(openCl);
    }
    v->addLayout(head);

    list_ = new QListWidget;
    list_->setViewMode(QListView::IconMode);
    list_->setResizeMode(QListView::Adjust);
    list_->setMovement(QListView::Static);
    list_->setIconSize(QSize(288, 162));
    list_->setGridSize(QSize(308, 214));
    list_->setUniformItemSizes(true);
    list_->setWordWrap(true);
    list_->setSpacing(4);
    list_->setStyleSheet("QListWidget{background:transparent;border:none;} QListWidget::item{border-radius:10px;padding:6px;}");
    v->addWidget(list_, 1);

    const QFileInfoList files = QDir(dir).entryInfoList({"*.mp4", "*.mkv", "*.mov"}, QDir::Files, QDir::Time);
    for (const QFileInfo& fi : files) {
        auto* it = new QListWidgetItem(tile(QImage(), -1),
                                       fi.completeBaseName() + "\n" + QLocale().toString(fi.lastModified(), QLocale::ShortFormat));
        it->setData(Qt::UserRole, fi.absoluteFilePath());
        it->setToolTip(fi.fileName());
        list_->addItem(it);
        queue_ << fi.absoluteFilePath();
    }
    if (files.isEmpty()) {
        auto* empty = new QLabel(T("no_clips"));
        empty->setAlignment(Qt::AlignCenter);
        empty->setObjectName("hint");
        v->addWidget(empty);
        list_->hide();
    }
    connect(list_, &QListWidget::itemActivated, this, [this](QListWidgetItem* it) {
        chosen_ = it->data(Qt::UserRole).toString();
        accept();
    });
    nextThumb();
}

// Vorschaubilder nacheinander per ffmpeg erzeugen (blockiert die Oberfläche nicht)
void ClipGalleryDialog::nextThumb() {
    if (proc_ || queue_.isEmpty() || !tmp_.isValid()) return;
    const QString file = queue_.takeFirst();
    const QString jpg = tmp_.filePath(QString::number(qHash(file)) + ".jpg");
    proc_ = new QProcess(this);
    connect(proc_, &QProcess::finished, this, [this, file, jpg] {
        const QString err = QString::fromUtf8(proc_->readAllStandardError());
        proc_->deleteLater();
        proc_ = nullptr;
        double dur = -1;
        auto m = QRegularExpression(R"(Duration:\s*(\d+):(\d+):(\d+(?:\.\d+)?))").match(err);
        if (m.hasMatch()) dur = m.captured(1).toInt() * 3600 + m.captured(2).toInt() * 60 + m.captured(3).toDouble();
        for (int i = 0; i < list_->count(); ++i)
            if (list_->item(i)->data(Qt::UserRole).toString() == file) list_->item(i)->setIcon(tile(QImage(jpg), dur));
        nextThumb();
    });
    proc_->start(ffmpegPath(), {"-hide_banner", "-y", "-ss", "1", "-i", file, "-frames:v", "1", "-vf", "scale=320:-2",
                                "-q:v", "5", jpg});
}
