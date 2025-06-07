#ifndef ANIMATEDTRIANGLE_H
#define ANIMATEDTRIANGLE_H

#include <QGraphicsPolygonItem>
#include <QPropertyAnimation>
#include <QColor>

class AnimatedTriangle : public QObject, public QGraphicsPolygonItem {
    Q_OBJECT
    // Definisci proprietà animabili
    Q_PROPERTY(qreal foldProgress READ foldProgress WRITE setFoldProgress)
    Q_PROPERTY(qreal currentOpacity READ currentOpacity WRITE setCurrentOpacity)

public:
    explicit AnimatedTriangle(const QPolygonF &polygon, const QColor &color = Qt::black, QGraphicsItem *parent = nullptr);

    void startFoldAnimation();
    void resetAnimation();

    // Getter e Setter per le proprietà animate
    qreal foldProgress() const { return m_foldProgress; }
    void setFoldProgress(qreal progress); // 0.0 = normale, 1.0 = piegato/svanito

    qreal currentOpacity() const { return m_currentOpacity; }
    void setCurrentOpacity(qreal opacity);

private:
    QPropertyAnimation *foldAnimation;
    QPropertyAnimation *opacityAnimation;

    QPolygonF m_originalPolygon;
    QColor m_originalColor;

    qreal m_foldProgress; // 0.0 to 1.0
    qreal m_currentOpacity; // 0.0 to 1.0

    // Metodo per aggiornare la forma del triangolo in base al foldProgress
    void updatePolygonShape();
};

#endif // ANIMATEDTRIANGLE_H