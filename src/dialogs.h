#pragma once
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
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
    QPushButton *ok_, *update_, *keys_;
};

// Knopf, der beim Anklicken die nächste Tastenkombination aufnimmt (Esc bricht ab, Rücktaste entfernt die Taste)
class KeyButton : public QPushButton {
    Q_OBJECT
public:
    explicit KeyButton(const QKeySequence& seq, QWidget* parent = nullptr);
    QKeySequence sequence() const { return seq_; }
    void setSequence(const QKeySequence& seq);

signals:
    void sequenceChanged(const QKeySequence& seq);

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;

private:
    void setRecording(bool on);
    QKeySequence seq_;
    bool recording_ = false;
};

// Alle Tastenkürzel ansehen und ändern - wirkt nach "OK" sofort
class ShortcutDialog : public QDialog {
    Q_OBJECT
public:
    explicit ShortcutDialog(QWidget* parent = nullptr);
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
    // Für die ungefähre Dateigröße: Länge des Ergebnisses, Original (Länge, Größe in Pixeln), Ton ja/nein
    void setEstimateInputs(double outDuration, double srcDuration, int natW, int natH, bool hasAudio);

private:
    void updateEstimate();
    int srcW_, srcH_;
    double srcFps_ = 30, outDur_ = 0, srcDur_ = 0, srcVideoBps_ = 0, srcPxRate_ = 0;
    class QProcess* probe_ = nullptr;  // Probe-Export (3 s) für eine echte Größenschätzung
    void showSize(double bits);
    bool hasAudio_ = true;
    QLabel* est_ = nullptr;
    QString source_;
    QString ext() const;
    QComboBox *format_, *res_, *fps_, *quality_;
    QLabel* ext_;
    QLineEdit *name_, *folder_;
    QPushButton* browse_;
    QCheckBox* replace_;
};
