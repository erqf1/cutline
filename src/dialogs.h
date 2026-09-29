#pragma once
#include <QComboBox>
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

private:
    void retranslate();
    QLabel *lblLang_, *lblTheme_;
    QComboBox *lang_, *theme_;
    QPushButton* ok_;
};

// Export-Optionen: Auflösung / Bildrate / Qualität – nur Werte bis zum Original werden angeboten
class ExportDialog : public QDialog {
    Q_OBJECT
public:
    ExportDialog(QWidget* parent, int srcW, int srcH, double srcFps);
    ExportOptions options() const;

private:
    int srcW_, srcH_;
    QComboBox *res_, *fps_, *quality_;
};
