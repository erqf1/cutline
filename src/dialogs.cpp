#include "dialogs.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include "i18n.h"
#include "theme.h"

// ---------------------------------------------------------------- Sprache (Erststart)
LanguageDialog::LanguageDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Language / Sprache");
    setMinimumSize(360, 460);
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(12);
    auto* title = new QLabel("Choose your language  ·  Sprache wählen");
    title->setObjectName("title");
    v->addWidget(title);
    list_ = new QListWidget;
    list_->addItems(languageNames());
    list_->setCurrentRow(1);
    v->addWidget(list_, 1);
    auto* ok = new QPushButton("OK");
    ok->setObjectName("primary");
    ok->setDefault(true);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    connect(list_, &QListWidget::itemDoubleClicked, this, &QDialog::accept);
    v->addWidget(ok);
}

QString LanguageDialog::code() const { return languageCodes().value(list_->currentRow(), "en"); }

// ---------------------------------------------------------------- Einstellungen
SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setMinimumWidth(380);
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(12);
    auto* form = new QFormLayout;
    form->setSpacing(12);
    lblLang_ = new QLabel;
    lblTheme_ = new QLabel;
    lang_ = new QComboBox;
    lang_->addItems(languageNames());
    lang_->setCurrentIndex(std::max<int>(0, languageCodes().indexOf(currentLanguage())));
    theme_ = new QComboBox;
    for (const Theme& t : themes()) theme_->addItem(t.name, t.id);
    theme_->setCurrentIndex(std::max<int>(0, theme_->findData(currentTheme().id)));
    form->addRow(lblLang_, lang_);
    form->addRow(lblTheme_, theme_);
    v->addLayout(form);
    update_ = new QPushButton;
    connect(update_, &QPushButton::clicked, this, &SettingsDialog::checkUpdates);
    auto* ver = new QLabel("Cutline " APP_VERSION);
    ver->setObjectName("hint");
    auto* ur = new QHBoxLayout;
    ur->addWidget(ver);
    ur->addStretch();
    ur->addWidget(update_);
    v->addLayout(ur);
    v->addStretch();
    ok_ = new QPushButton;
    ok_->setObjectName("primary");
    connect(ok_, &QPushButton::clicked, this, &QDialog::accept);
    v->addWidget(ok_);

    connect(lang_, &QComboBox::currentIndexChanged, this, [this](int i) {
        emit languageChanged(languageCodes().value(i, "en"));
        retranslate();
    });
    connect(theme_, &QComboBox::currentIndexChanged, this,
            [this] { emit themeChanged(theme_->currentData().toString()); });
    retranslate();
}

void SettingsDialog::retranslate() {
    setWindowTitle(T("settings"));
    lblLang_->setText(T("language"));
    lblTheme_->setText(T("theme"));
    ok_->setText(T("ok"));
    update_->setText(T("upd_check"));
}

// ---------------------------------------------------------------- Export
ExportDialog::ExportDialog(QWidget* parent, int srcW, int srcH, double srcFps, const QString& sourcePath)
    : QDialog(parent), srcW_(srcW), srcH_(srcH), source_(sourcePath) {
    setWindowTitle(T("export"));
    setMinimumWidth(420);
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(12);
    auto* form = new QFormLayout;
    form->setSpacing(12);

    res_ = new QComboBox;
    res_->addItem(QString("%1 (%2×%3)").arg(T("original")).arg(srcW).arg(srcH), srcH);
    struct R { int h; const char* name; };
    for (R r : {R{2160, "4K (2160p)"}, R{1440, "2K (1440p)"}, R{1080, "Full HD (1080p)"}, R{720, "HD (720p)"},
                R{480, "SD (480p)"}, R{360, "SD (360p)"}, R{240, "240p"}})
        if (r.h < srcH) res_->addItem(r.name, r.h);
    form->addRow(T("resolution"), res_);

    fps_ = new QComboBox;
    const double f0 = srcFps > 1 ? srcFps : 30.0;
    fps_->addItem(QString("%1 (%2 fps)").arg(T("original")).arg(f0, 0, 'g', 5), f0);
    for (double f : {60.0, 50.0, 30.0, 25.0, 24.0, 15.0})
        if (f < f0 - 0.5) fps_->addItem(QString("%1 fps").arg(f, 0, 'g', 3), f);
    form->addRow(T("framerate"), fps_);

    quality_ = new QComboBox;
    quality_->addItem(T("q_high"), 0);
    quality_->addItem(T("q_mid"), 1);
    quality_->addItem(T("q_small"), 2);
    quality_->setCurrentIndex(1);
    form->addRow(T("quality"), quality_);
    v->addLayout(form);

    auto* hint = new QLabel(T("export_hint"));
    hint->setObjectName("hint");
    hint->setWordWrap(true);
    v->addWidget(hint);

    // Ziel: Name + Speicherort (mit Durchsuchen) oder Originaldatei ersetzen
    v->addSpacing(6);
    auto* dest = new QLabel(T("save_to"));
    dest->setObjectName("title");
    v->addWidget(dest);
    auto* form2 = new QFormLayout;
    form2->setSpacing(10);
    const QFileInfo src(source_);
    name_ = new QLineEdit(src.completeBaseName() + "_edit");
    auto* nameRow = new QHBoxLayout;
    nameRow->addWidget(name_, 1);
    nameRow->addWidget(new QLabel(".mp4"));
    form2->addRow(T("file_name"), nameRow);
    folder_ = new QLineEdit(QDir::toNativeSeparators(src.absolutePath()));
    browse_ = new QPushButton(T("browse"));
    connect(browse_, &QPushButton::clicked, this, [this] {
        const QString d = QFileDialog::getExistingDirectory(this, T("folder"), folder_->text());
        if (!d.isEmpty()) folder_->setText(QDir::toNativeSeparators(d));
    });
    auto* folderRow = new QHBoxLayout;
    folderRow->addWidget(folder_, 1);
    folderRow->addWidget(browse_);
    form2->addRow(T("folder"), folderRow);
    v->addLayout(form2);
    replace_ = new QCheckBox(T("replace_orig"));
    replace_->setToolTip(T("replace_hint"));
    auto* rhint = new QLabel(T("replace_hint"));
    rhint->setObjectName("hint");
    rhint->setWordWrap(true);
    rhint->hide();
    v->addWidget(replace_);
    v->addWidget(rhint);
    const QString savedName = name_->text();
    connect(replace_, &QCheckBox::toggled, this, [this, rhint, savedName](bool on) {
        const QFileInfo fi(source_);
        name_->setText(on ? fi.completeBaseName() : savedName);
        folder_->setText(QDir::toNativeSeparators(fi.absolutePath()));
        name_->setEnabled(!on);
        folder_->setEnabled(!on);
        browse_->setEnabled(!on);
        rhint->setVisible(on);
    });

    auto* row = new QHBoxLayout;
    auto* cancel = new QPushButton(T("cancel"));
    auto* ok = new QPushButton(T("export"));
    ok->setObjectName("primary");
    ok->setDefault(true);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    row->addStretch();
    row->addWidget(cancel);
    row->addWidget(ok);
    v->addLayout(row);
}

ExportOptions ExportDialog::options() const {
    ExportOptions o;
    int h = res_->currentData().toInt();
    h -= h % 2;
    int w = int(std::lround(double(srcW_) * h / srcH_ / 2.0)) * 2;
    o.width = std::max(2, w);
    o.height = std::max(2, h);
    o.fps = fps_->currentData().toDouble();
    o.quality = quality_->currentData().toInt();
    o.encoder = detectEncoder();
    return o;
}

QString ExportDialog::outputPath() const {
    QString n = name_->text().trimmed();
    if (n.endsWith(".mp4", Qt::CaseInsensitive)) n.chop(4);
    return QDir(QDir::fromNativeSeparators(folder_->text().trimmed())).filePath(n + ".mp4");
}

bool ExportDialog::replaceOriginal() const { return replace_->isChecked(); }

void ExportDialog::accept() {
    if (name_->text().trimmed().isEmpty()) { name_->setFocus(); return; }
    if (!QDir(QDir::fromNativeSeparators(folder_->text().trimmed())).exists()) { folder_->setFocus(); return; }
    const QString out = outputPath();
    if (!replaceOriginal() && QFileInfo::exists(out) &&
        QMessageBox::question(this, T("export"), T("overwrite_q").arg(QFileInfo(out).fileName())) != QMessageBox::Yes)
        return;
    QDialog::accept();
}
