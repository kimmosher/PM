#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // --- Create splash screen ---
    QPixmap pix(":/resources/splash.png");
    QSplashScreen splash(pix);
    splash.show();

    // Force the splash to paint immediately
    app.processEvents();

    // --- Create main window, but don't show it yet ---
    MainWindow w;

    // --- After 3 seconds, hide splash and show main window ---
    QTimer::singleShot(4000, [&]() {
        splash.close();
        w.show();
    });

    return app.exec();
}
