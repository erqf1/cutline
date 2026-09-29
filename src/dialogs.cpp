#include "dialogs.h"

#include <QFormLayout>
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
}

// ---------------------------------------------------------------- Export
ExportDialog::ExportDialog(QWidget* parent, int srcW, int srcH, double srcFps)
    : QDialog(parent), srcW_(srcW), srcH_(srcH) {
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
