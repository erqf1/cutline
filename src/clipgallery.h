#pragma once
#include <QDialog>
#include <QListWidget>
#include <QProcess>
#include <QTemporaryDir>

// Clips aus Clipline (falls installiert). QSettings("WSoftware", "Clipline") liefert Ordner + Programmpfad.
QString cliplineClipsDir();   // leer = Clipline nicht eingerichtet
QString cliplineExe();

class ClipGalleryDialog : public QDialog {
    Q_OBJECT
public:
    explicit ClipGalleryDialog(QWidget* parent = nullptr);
    QString chosen() const { return chosen_; }

private:
    void nextThumb();

    QListWidget* list_;
    QStringList queue_;
    QProcess* proc_ = nullptr;
    QTemporaryDir tmp_;
    QString chosen_;
};
