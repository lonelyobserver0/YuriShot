#include "screenshotanimator.h"
#include "animatedtriangle.h"
#include <QGraphicsPolygonItem>
#include <QTimer>
#include <QDebug>
#include <cmath>
#include <QVBoxLayout>
#include <QApplication>
#include <QEvent> // *** AGGIUNGI QUESTO INCLUDE ***

ScreenshotAnimator::ScreenshotAnimator(QWidget *parent) : QWidget(parent) {
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFrameStyle(QFrame::NoFrame);
    view->setRenderHint(QPainter::Antialiasing);

    view->setMouseTracking(true);
    view->setInteractive(false); // Disabilita l'interazione standard del QGraphicsView

    // *** INSTALLA L'EVENT FILTER SUL VIEWPORT DEL QGRAPHICSVIEW ***
    // Il viewport è la parte effettiva del QGraphicsView che disegna la scena
    view->viewport()->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view);
    layout->setContentsMargins(0, 0, 0, 0);

    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    selecting = false;

    QTimer::singleShot(0, this, [this]() {
        scene->setSceneRect(this->rect());
        setupTriangles();
    });

    qDebug() << "ScreenshotAnimator initialized.";
}

ScreenshotAnimator::~ScreenshotAnimator() {
    // Non è necessario disinstallare l'event filter esplicitamente
    // se l'oggetto filtro (this) viene eliminato prima dell'oggetto filtrato (view->viewport()).
    // In questo caso, view->viewport() è figlio di view che è figlio di this.
    // L'ordine di distruzione assicura che sia corretto.
    delete rubberBand;
}

void ScreenshotAnimator::setupTriangles() {
    // Dimensione approssimativa di ogni lato del triangolo (per una griglia)
    const int triangleSize = 50;
    const QRect sceneRect = scene->sceneRect().toRect();

    // Riempi la scena con una griglia di triangoli
    for (int y = sceneRect.top(); y < sceneRect.bottom(); y += triangleSize / 2) {
        for (int x = sceneRect.left(); x < sceneRect.right(); x += triangleSize) {
            // Primo triangolo del "quadrato"
            QPolygonF triangle1;
            triangle1 << QPointF(x, y)
                      << QPointF(x + triangleSize, y)
                      << QPointF(x + triangleSize / 2, y + triangleSize / 2);
            
            QPolygonF triangle2;
            triangle2 << QPointF(x, y + triangleSize / 2)
                      << QPointF(x + triangleSize / 2, y + triangleSize / 2)
                      << QPointF(x, y);

            // Ogni triangolo è un'istanza di AnimatedTriangle
            AnimatedTriangle *tri1 = new AnimatedTriangle(triangle1, Qt::black);
            scene->addItem(tri1);

            AnimatedTriangle *tri2 = new AnimatedTriangle(triangle2, Qt::black);
            scene->addItem(tri2);
        }
    }
    qDebug() << "Triangoli creati:" << scene->items().count();
}

// *** IMPLEMENTAZIONE DELL'EVENT FILTER ***
bool ScreenshotAnimator::eventFilter(QObject *obj, QEvent *event) {
    // Controlla se l'evento proviene dal viewport del QGraphicsView
    if (obj == view->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            // Reindirizza l'evento al tuo metodo mousePressEvent
            this->mousePressEvent(static_cast<QMouseEvent*>(event));
            return true; // Consuma l'evento, non lasciarlo propagare oltre
        } else if (event->type() == QEvent::MouseMove) {
            this->mouseMoveEvent(static_cast<QMouseEvent*>(event));
            return true; // Consuma l'evento
        } else if (event->type() == QEvent::MouseButtonRelease) {
            this->mouseReleaseEvent(static_cast<QMouseEvent*>(event));
            return true; // Consuma l'evento
        }
    }
    // Per tutti gli altri oggetti o tipi di eventi, lascia che l'implementazione base li gestisca
    return QWidget::eventFilter(obj, event);
}

void ScreenshotAnimator::mousePressEvent(QMouseEvent *event) {
    qDebug() << "Mouse Pressed at:" << event->pos(); // DEBUG
    if (event->button() == Qt::LeftButton) {
        originPoint = event->pos();
        rubberBand->setGeometry(QRect(originPoint, QSize()));
        rubberBand->show();
        selecting = true;
    }
    // *** Rimuovi la chiamata a QWidget::mousePressEvent(event); ***
    // L'eventFilter ha già gestito e consumato l'evento.
}

void ScreenshotAnimator::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << "Mouse Moved to:" << event->pos(); // DEBUG
    if (selecting) {
        rubberBand->setGeometry(QRect(originPoint, event->pos()).normalized());
        animateTrianglesInRegion(rubberBand->geometry());
    }
    // *** Rimuovi la chiamata a QWidget::mouseMoveEvent(event); ***
}

void ScreenshotAnimator::mouseReleaseEvent(QMouseEvent *event) {
    qDebug() << "Mouse Released at:" << event->pos(); // DEBUG
    if (selecting && event->button() == Qt::LeftButton) {
        selecting = false;
        rubberBand->hide();

        QRect selectedRect = QRect(originPoint, event->pos()).normalized();

        if (selectedRect.width() < 1) selectedRect.setWidth(1);
        if (selectedRect.height() < 1) selectedRect.setHeight(1);
        
        qDebug() << "Regione selezionata:" << selectedRect;
        
        launchFlameshot(selectedRect);

        // --- NEW: Reset all triangles or remove them ---
        // Option A: Reset all animations (they will become visible again)
        for (QGraphicsItem *item : scene->items()) {
            if (AnimatedTriangle *triangle = dynamic_cast<AnimatedTriangle*>(item)) {
                // If you want them to completely disappear, you could stop and set opacity to 0
                // triangle->setOpacity(0.0); // This would make them instantly disappear
                // Or:
                triangle->resetAnimation(); // They will fade back in
            }
        }
    }
    // *** Rimuovi la chiamata a QWidget::mouseReleaseEvent(event); ***
}

void ScreenshotAnimator::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "ESC premuto, chiusura dell'animatore.";
        QApplication::quit(); // Chiude l'applicazione
    }
    QWidget::keyPressEvent(event); // Chiamata al metodo base
}

void ScreenshotAnimator::animateTrianglesInRegion(const QRect &region) {
    // Questa è la logica centrale per la tua animazione personalizzata.
    // Itera su tutti i triangoli nella scena
    for (QGraphicsItem *item : scene->items()) {
        AnimatedTriangle *triangle = dynamic_cast<AnimatedTriangle*>(item);
        if (triangle) {
            // Controlla se il triangolo interseca la regione o è nelle vicinanze
            // Calcola una "zona di influenza" attorno alla regione di selezione
            QRect influenceRect = region.adjusted(-50, -50, 50, 50); // Esempio: 50px di margine
            
            if (triangle->boundingRect().intersects(influenceRect)) {
                // Attiva l'animazione di "piegatura" per questo triangolo
                // Puoi passare la posizione del mouse o il centro della selezione per un effetto direzionale
                triangle->startFoldAnimation();
            } else {
                // Se il triangolo è fuori dalla zona di influenza, riportalo al suo stato originale
                // (se non è già completamente svanito o animato)
                triangle->resetAnimation();
            }
        }
    }
}

void ScreenshotAnimator::launchFlameshot(const QRect &region) {
    QString regionStr = QString("%1,%2,%3,%4")
                            .arg(region.x())
                            .arg(region.y())
                            .arg(region.width())
                            .arg(region.height());

    QStringList arguments;
    arguments << "gui" << "--region" << regionStr;

    qDebug() << "Lancio Flameshot con:" << arguments.join(" ");

    // *** NASCONDI LA FINESTRA PRIMA DI LANCIARE FLAMESHOT ***
    this->hide(); // Nasconde la finestra ScreenshotAnimator

    QProcess *flameshotProcess = new QProcess(this);
    connect(flameshotProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "Flameshot terminato con codice:" << exitCode;
        // Mostra di nuovo la finestra solo se necessario per future interazioni,
        // altrimenti chiudi l'app direttamente.
        // this->show(); // Se vuoi che riappaia per un altro screenshot
        this->close(); // Chiudi l'animatore dopo che Flameshot ha fatto il suo lavoro
        // O QApplication::quit(); per terminare l'intera app se non hai altre finestre
    });
    flameshotProcess->start("flameshot", arguments);

    if (!flameshotProcess->waitForStarted(5000)) { // Attendi fino a 5 secondi per l'avvio
        qWarning() << "Errore: Flameshot non è riuscito ad avviarsi.";
        this->close(); // Chiudi se Flameshot non si avvia
    }
}
