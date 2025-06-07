#include "animatedtriangle.h"
#include <QDebug>
#include <QGraphicsEffect> // Potresti usarlo per effetti aggiuntivi
#include <QBrush>
#include <QPen>

AnimatedTriangle::AnimatedTriangle(const QPolygonF &polygon, const QColor &color, QGraphicsItem *parent)
    : QObject(nullptr), QGraphicsPolygonItem(polygon, parent),
      m_originalPolygon(polygon), m_originalColor(color),
      m_foldProgress(0.0), m_currentOpacity(1.0) {
    
    setBrush(QBrush(m_originalColor));
    setPen(QPen(Qt::NoPen)); // Senza bordo per default

    // Inizializza le animazioni
    foldAnimation = new QPropertyAnimation(this, "foldProgress");
    foldAnimation->setDuration(300); // 300 ms
    foldAnimation->setEasingCurve(QEasingCurve::OutQuad); // Curva di accelerazione

    opacityAnimation = new QPropertyAnimation(this, "currentOpacity");
    opacityAnimation->setDuration(300);
    opacityAnimation->setEasingCurve(QEasingCurve::OutQuad);

    // Connetti il segnale animation progress all'aggiornamento della forma
    connect(foldAnimation, &QPropertyAnimation::valueChanged, this, &AnimatedTriangle::updatePolygonShape);
    // Connetti l'aggiornamento dell'opacità alla proprietà opacity di QGraphicsItem
    connect(opacityAnimation, &QPropertyAnimation::valueChanged, this, [this](const QVariant &value){
        setOpacity(value.toReal());
    });
}

void AnimatedTriangle::startFoldAnimation() {
    if (foldAnimation->state() != QAbstractAnimation::Running && m_foldProgress < 1.0) {
        foldAnimation->setStartValue(m_foldProgress);
        foldAnimation->setEndValue(1.0);
        foldAnimation->start();

        opacityAnimation->setStartValue(m_currentOpacity);
        opacityAnimation->setEndValue(0.0); // Svanisce
        opacityAnimation->start();
    }
}

void AnimatedTriangle::resetAnimation() {
    if (foldAnimation->state() != QAbstractAnimation::Running && m_foldProgress > 0.0) {
        foldAnimation->setStartValue(m_foldProgress);
        foldAnimation->setEndValue(0.0);
        foldAnimation->start();

        opacityAnimation->setStartValue(m_currentOpacity);
        opacityAnimation->setEndValue(1.0); // Ritorna visibile
        opacityAnimation->start();
    }
}

void AnimatedTriangle::setFoldProgress(qreal progress) {
    if (m_foldProgress != progress) {
        m_foldProgress = progress;
        updatePolygonShape(); // Chiamata per aggiornare la geometria
    }
}

void AnimatedTriangle::setCurrentOpacity(qreal opacity) {
    if (m_currentOpacity != opacity) {
        m_currentOpacity = opacity;
        // setOpacity() è un metodo di QGraphicsItem
        // Questo sarà chiamato dal connect nel costruttore
    }
}

void AnimatedTriangle::updatePolygonShape() {
    // Questa è la parte cruciale per l'effetto "piegatura".
    // Devi manipolare i vertici di `m_originalPolygon` in base a `m_foldProgress`.
    // L'animazione da te descritta ("si piegano su se stessi sparendo") suggerisce:
    // 1. I vertici si muovono verso un punto centrale o un lato del triangolo.
    // 2. Il triangolo si appiattisce o si rimpicciolisce.
    // 3. L'opacità diminuisce (gestita da opacityAnimation).

    QPolygonF currentPolygon = m_originalPolygon;

    // Esempio SEMPLICE di animazione di "piegatura" (rimpittolimento verso il centro)
    // Non è esattamente una "piegatura" 3D, ma un'illusione 2D.
    // Il punto centrale del triangolo
    QPointF center = m_originalPolygon.boundingRect().center();

    for (int i = 0; i < currentPolygon.size(); ++i) {
        QPointF original_vertex = m_originalPolygon[i];
        
        // Interpola la posizione del vertice dal suo originale verso il centro
        // Più foldProgress è vicino a 1, più il vertice si avvicina al centro
        qreal x = original_vertex.x() + (center.x() - original_vertex.x()) * m_foldProgress;
        qreal y = original_vertex.y() + (center.y() - original_vertex.y()) * m_foldProgress;
        currentPolygon[i] = QPointF(x, y);
    }
    
    // Imposta il poligono aggiornato per il QGraphicsPolygonItem
    setPolygon(currentPolygon);
}