#ifndef SCREENSHOTANIMATOR_H
#define SCREENSHOTANIMATOR_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QRubberBand>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QProcess>
#include <QEvent> // *** AGGIUNGI QUESTO INCLUDE ***

// Dichiarazione forward
class AnimatedTriangle;

class ScreenshotAnimator : public QWidget {
    Q_OBJECT

public:
    explicit ScreenshotAnimator(QWidget *parent = nullptr);
    ~ScreenshotAnimator();

protected:
    // I tuoi metodi originali di gestione eventi mouse/tastiera rimangono.
    // Li chiameremo da eventFilter.
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    // *** AGGIUNGI LA DICHIARAZIONE DELL'EVENT FILTER ***
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QGraphicsView *view;
    QGraphicsScene *scene;
    QRubberBand *rubberBand;

    QPoint originPoint;
    bool selecting;

    void setupTriangles();
    void animateTrianglesInRegion(const QRect &region);
    void launchFlameshot(const QRect &region);
};

#endif // SCREENSHOTANIMATOR_H
