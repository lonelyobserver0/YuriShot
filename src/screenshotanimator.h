#ifndef SCREENSHOTANIMATOR_H
#define SCREENSHOTANIMATOR_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QRubberBand>
#include <QMouseEvent>
#include <QKeyEvent> // Per ESC
#include <QProcess> // Per lanciare Flameshot

// Dichiarazione forward
class AnimatedTriangle;

class ScreenshotAnimator : public QWidget {
    Q_OBJECT

public:
    explicit ScreenshotAnimator(QWidget *parent = nullptr);
    ~ScreenshotAnimator();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QGraphicsView *view;
    QGraphicsScene *scene;
    QRubberBand *rubberBand; // Useremo un QRubberBand per il feedback visivo della selezione
                               // MA lo sostituiremo con la nostra animazione dei triangoli
    
    QPoint originPoint;
    bool selecting;

    // Metodo per popolare la scena con i triangoli
    void setupTriangles();
    // Metodo per animare i triangoli in una data regione
    void animateTrianglesInRegion(const QRect &region);
    // Metodo per lanciare Flameshot
    void launchFlameshot(const QRect &region);
};

#endif // SCREENSHOTANIMATOR_H
