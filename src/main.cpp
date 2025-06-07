#include <QApplication>
#include <QScreen>
#include "screenshotanimator.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ScreenshotAnimator animator;

    // Imposta la finestra a schermo intero e senza bordi
    animator.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Window);
    animator.setAttribute(Qt::WA_TranslucentBackground);
    animator.setAttribute(Qt::WA_NoSystemBackground); // Importante per la trasparenza su alcuni WM/compositor

    // Prendi le dimensioni dello schermo principale
    QScreen *screen = app.primaryScreen();
    QRect screenGeometry = screen->geometry();
    animator.setGeometry(screenGeometry); // Rende la finestra a schermo intero

    animator.show();

    return app.exec();
}
