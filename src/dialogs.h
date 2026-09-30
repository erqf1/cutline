#pragma once
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include "model.h"

// Erststart: Sprache wählen
class LanguageDialog : public QDialog {
    Q_OBJECT
public:
    explicit LanguageDialog(QWidget* parent = nullptr);
    QString code() const;

private:
    QListWidget* list_;
};

// Einstellungen: Sprache + Design (wirkt sofort)
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

signals:
    void languageChanged(const QString& code);
    void themeChanged(const QString& id);
    void checkUpdates();

private:
    void retranslate();
    QLabel *lblLang_, *lblTheme_;
    QComboBox *lang_, *theme_;
    QPushButton *ok_, *update_;
};

// Export-Optionen: Auflösung / Bildrate / Qualität – nur Werte bis zum Original werden angeboten
class ExportDialog : public QDialog {
    Q_OBJECT
public:
    ExportDialog(QWidget* parent, int srcW, int srcH, double srcFps, const QString& sourcePath);
    ExportOptions options() const;
    QString outputPath() const;      // Ziel (bei "Original ersetzen": der spätere Dateiname)
    bool replaceOriginal() const;
    void accept() override;

private:
    int srcW_, srcH_;
    QString source_;
    QString ext() const;
    QComboBox *format_, *res_, *fps_, *quality_;
    QLabel* ext_;
    QLineEdit *name_, *folder_;
    QPushButton* browse_;
    QCheckBox* replace_;
};
