// main.cpp
#include <QApplication>
#include <QScreen>
#include "screenshotanimator.h"
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Importante per Wayland: Richiedi decorazioni lato client
    // Questo dice a Qt di non chiedere al compositor di disegnare le decorazioni.
    // Il compositor (Hyprland) vedrà che la finestra non vuole decorazioni.
    app.setAttribute(Qt::AA_DontUseNativeMenuBar); // Potrebbe essere utile per alcune app
    app.setAttribute(Qt::AA_EnableHighDpiScaling); // Raccomandato per Wayland/HiDPI
    
    ScreenshotAnimator animator;

    // Imposta la finestra a schermo intero e senza bordi
    // Qt::Window è il tipo di finestra top-level per Wayland
    animator.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window); 

    animator.setAttribute(Qt::WA_TranslucentBackground);
    // WA_NoSystemBackground è meno critico su Wayland rispetto a X11, ma non fa male
    animator.setAttribute(Qt::WA_NoSystemBackground); 

    // Prendi le dimensioni dello schermo principale
    QScreen *screen = app.primaryScreen();
    if (!screen) {
        qWarning() << "No primary screen found!"; // This line caused the error
        return -1;
    }
    QRect screenGeometry = screen->geometry();
    animator.setGeometry(screenGeometry); // Rende la finestra a schermo intero

    animator.show();

    qDebug() << "Animator window ID (for Hyprland rule):" << animator.winId(); // This line caused the error
    qDebug() << "Animator class name (for Hyprland rule):" << animator.metaObject()->className(); // This line caused the error

    return app.exec();
}
