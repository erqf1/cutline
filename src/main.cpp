#include <QtGlobal>
#ifdef Q_OS_WIN
#include <windows.h>
#include <shobjidl.h>
#endif
#include <QApplication>
#include <QFileOpenEvent>
#include <QGuiApplication>
#include <QSettings>
#include <QTimer>
#include "dialogs.h"
#include "i18n.h"
#include "icons.h"
#include "mainwindow.h"
#include "textrender.h"
#include "theme.h"

// macOS liefert "Oeffnen mit"/Doppelklick als QFileOpenEvent statt als Argument
class App : public QApplication {
public:
    using QApplication::QApplication;
    MainWindow* window = nullptr;
    QString pending;

protected:
    bool event(QEvent* e) override {
        if (e->type() == QEvent::FileOpen) {
            const QString f = static_cast<QFileOpenEvent*>(e)->file();
            if (window) window->openFile(f); else pending = f;
            return true;
        }
        return QApplication::event(e);
    }
};

int main(int argc, char* argv[]) {
#ifdef Q_OS_WIN
    // Eigene App-ID: eigener Taskleisten-Button, nicht mit anderen Programmen gruppiert
    SetCurrentProcessExplicitAppUserModelID(L"WSoftware.Cutline");
#endif
#ifdef Q_OS_LINUX
    // Linux, vor allem integrierte Grafik unter Wayland (z. B. Hyprland): Hardware-Dekodierung (VAAPI) und die
    // GPU-Umrechnung der Videobilder lassen Qt Multimedia dort oft abstürzen. Software-Dekodierung reicht für
    // 1080p locker. Wer sie trotzdem will: CUTLINE_HW_DECODE=1. Eigene Werte der Variablen bleiben unangetastet.
    if (qEnvironmentVariable("CUTLINE_HW_DECODE") != "1") {
        if (!qEnvironmentVariableIsSet("QT_FFMPEG_DECODING_HW_DEVICE_TYPES"))
            qputenv("QT_FFMPEG_DECODING_HW_DEVICE_TYPES", ",");  // gesetzt, aber leer -> keine Hardware-Geräte
        if (!qEnvironmentVariableIsSet("QT_DISABLE_HW_TEXTURES_CONVERSION"))
            qputenv("QT_DISABLE_HW_TEXTURES_CONVERSION", "1");
    }
#endif
    App app(argc, argv);
    registerSpecialFonts();
    QCoreApplication::setOrganizationName("WSoftware");
    QCoreApplication::setApplicationName("VideoEditor");
    QGuiApplication::setApplicationDisplayName("Cutline");

    QIcon appIcon;
    for (int s : {16, 24, 32, 48, 64, 128, 256}) appIcon.addPixmap(makeAppPixmap(s));
    app.setWindowIcon(appIcon);

    QSettings st;
    setCurrentTheme(st.value("theme", "mint").toString());
    app.setStyleSheet(buildStyleSheet(currentTheme()));

    QString lang = st.value("language").toString();
    if (lang.isEmpty()) {  // Erststart: Sprache abfragen
        LanguageDialog dlg;
        dlg.exec();
        lang = dlg.code();
        st.setValue("language", lang);
    }
    setLanguage(lang);

    MainWindow w;
    w.show();
    app.window = &w;
    if (!app.pending.isEmpty()) w.openFile(app.pending);
    // Argumente als Unicode lesen (Dateien aus "Oeffnen mit" koennen Sonderzeichen enthalten)
    const QStringList args = app.arguments();
    if (args.size() > 1 && !args[1].startsWith("--"))
        QTimer::singleShot(0, &w, [&w, path = args[1]] { w.openFile(path); });
    if (args.size() > 3 && args[2] == "--shots") w.selfShots(args[3]);
    if (args.size() > 3 && args[2] == "--aspectshots") w.aspectShots(args[3]);
    if (args.size() > 3 && args[2] == "--fsshots") w.fsShots(args[3]);
    return app.exec();
}
