#include <QApplication>
#include <QGuiApplication>
#include <QSettings>
#include <QTimer>
#include "dialogs.h"
#include "i18n.h"
#include "icons.h"
#include "mainwindow.h"
#include "theme.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
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
    if (argc > 1) QTimer::singleShot(0, &w, [&w, path = QString::fromLocal8Bit(argv[1])] { w.openFile(path); });
    if (argc > 3 && QString(argv[2]) == "--shots") w.selfShots(QString::fromLocal8Bit(argv[3]));
    return app.exec();
}
