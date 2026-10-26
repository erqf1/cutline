#include "fileicons.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#endif

static QString iconDir(const QString& variant) {
    return QCoreApplication::applicationDirPath() + "/filetypes/" + variant;
}

bool fileIconsAvailable() {
#ifdef Q_OS_WIN
    if (!QFileInfo(iconDir("light")).isDir()) return false;
    // Nur, wenn der Installer die Dateitypen eingetragen hat (nicht bei der portablen Version)
    const QSettings reg("HKEY_CLASSES_ROOT\\Cutline.mp3\\shell\\open\\command", QSettings::NativeFormat);
    return reg.contains("Default");
#else
    return false;
#endif
}

QString fileIconsMode() { return QSettings().value("fileIcons", "auto").toString(); }

static QString resolved(const QString& mode) {
    if (mode == "light" || mode == "dark") return mode;
#ifdef Q_OS_WIN
    // Windows-Modus für Apps (wie der Explorer): AppsUseLightTheme = 0 -> dunkel
    const QSettings p("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    return p.value("AppsUseLightTheme", 1).toInt() == 0 ? "dark" : "light";
#else
    return "light";
#endif
}

static bool apply(const QString& variant) {
#ifdef Q_OS_WIN
    const QFileInfoList icons = QDir(iconDir(variant)).entryInfoList({"*.ico"}, QDir::Files);
    if (icons.isEmpty()) return false;
    for (const QFileInfo& f : icons) {
        QSettings reg("HKEY_CURRENT_USER\\Software\\Classes\\Cutline." + f.completeBaseName() + "\\DefaultIcon", QSettings::NativeFormat);
        reg.setValue("Default", QDir::toNativeSeparators(f.absoluteFilePath()));
    }
    // "Video" und "Audio": Sammel-ProgIDs älterer Versionen
    for (const char* kind : {"Video", "Audio"}) {
        QSettings reg(QString("HKEY_CURRENT_USER\\Software\\Classes\\Cutline.%1\\DefaultIcon").arg(kind), QSettings::NativeFormat);
        reg.setValue("Default", QDir::toNativeSeparators(iconDir(variant) + "/" + QString(kind).toLower() + ".ico"));
    }
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);  // Explorer zeichnet die Symbole neu
    QSettings().setValue("fileIconsApplied", variant);
    return true;
#else
    Q_UNUSED(variant);
    return false;
#endif
}

void setFileIconsMode(const QString& mode) {
    QSettings().setValue("fileIcons", mode);
    if (fileIconsAvailable()) apply(resolved(mode));
}

void syncFileIcons() {
    if (!fileIconsAvailable()) return;
    const QString want = resolved(fileIconsMode());
    // Hell trägt der Installer selbst ein; dunkel nach jedem Update neu setzen (der Installer schreibt wieder hell)
    if (want == "light" && QSettings().value("fileIconsApplied", "light").toString() == "light") return;
    apply(want);
}
