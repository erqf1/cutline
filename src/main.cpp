#include <QApplication>
#include <QFileOpenEvent>
#include <QGuiApplication>
#include <QSettings>
#include <QTimer>
#include "dialogs.h"
#include "i18n.h"
#include "icons.h"
#include "mainwindow.h"
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
    App app(argc, argv);
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
    return app.exec();
}
