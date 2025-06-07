#include "screenshotanimator.h"
#include "animatedtriangle.h" // Includi la nuova classe AnimatedTriangle
#include <QGraphicsPolygonItem>
#include <QTimer>
#include <QDebug>
#include <cmath> // Per std::hypot
#include <QVBoxLayout> // Per QVBoxLayout
#include <QApplication> // Per QApplication::quit()

ScreenshotAnimator::ScreenshotAnimator(QWidget *parent) : QWidget(parent) {
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFrameStyle(QFrame::NoFrame); // Rimuove il bordo della view
    view->setRenderHint(QPainter::Antialiasing); // Migliora la qualità di rendering

    // Imposta la view per occupare tutta la finestra
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view);
    layout->setContentsMargins(0, 0, 0, 0);

    // QRubberBand è solo un segnaposto per la visualizzazione della selezione,
    // la tua animazione personalizzata la sostituirà visivamente.
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    selecting = false;

    // Imposta la scena con le dimensioni dello schermo una volta che la finestra è visibile
    QTimer::singleShot(0, this, [this]() {
        scene->setSceneRect(this->rect());
        setupTriangles(); // Popola la scena con i triangoli
    });
}

ScreenshotAnimator::~ScreenshotAnimator() {
    delete rubberBand;
    // scene e view vengono eliminati automaticamente perché figli del QWidget
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
                      << QPointF(x + triangleSize / 2, y + triangleSize / 2); // Vertice in basso al centro
            
            // Secondo triangolo del "quadrato" (capovolto o adiacente)
            QPolygonF triangle2;
            triangle2 << QPointF(x, y + triangleSize / 2)
                      << QPointF(x + triangleSize / 2, y + triangleSize / 2)
                      << QPointF(x, y); // Esempio semplice, puoi variare la tassellazione

            // Più complesso: una tassellazione di triangoli equilateri o rettangoli divisi
            // Per semplicità, creiamo solo alcuni per mostrare il concetto
            
            // Ogni triangolo è un'istanza di AnimatedTriangle
            // Nota: Colore iniziale nero semi-trasparente
            AnimatedTriangle *tri1 = new AnimatedTriangle(triangle1, Qt::black);
            scene->addItem(tri1);

            AnimatedTriangle *tri2 = new AnimatedTriangle(triangle2, Qt::black);
            scene->addItem(tri2);
        }
    }
    qDebug() << "Triangoli creati:" << scene->items().count();
}

void ScreenshotAnimator::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        originPoint = event->pos();
        rubberBand->setGeometry(QRect(originPoint, QSize()));
        rubberBand->show();
        selecting = true;
        // Inizia a nascondere i triangoli nell'area circostante
    }
}

void ScreenshotAnimator::mouseMoveEvent(QMouseEvent *event) {
    if (selecting) {
        // Aggiorna le dimensioni del rubberBand per mostrare la selezione.
        // La tua animazione personalizzata dei triangoli dovrà reagire a questo rettangolo.
        rubberBand->setGeometry(QRect(originPoint, event->pos()).normalized());
        
        // Attiva l'animazione dei triangoli nell'area di selezione corrente
        animateTrianglesInRegion(rubberBand->geometry());
    }
}

void ScreenshotAnimator::mouseReleaseEvent(QMouseEvent *event) {
    if (selecting && event->button() == Qt::LeftButton) {
        selecting = false;
        rubberBand->hide();

        QRect selectedRect = QRect(originPoint, event->pos()).normalized();

        // Assicurati che le dimensioni siano valide
        if (selectedRect.width() < 1) selectedRect.setWidth(1);
        if (selectedRect.height() < 1) selectedRect.setHeight(1);
        
        qDebug() << "Regione selezionata:" << selectedRect;
        
        launchFlameshot(selectedRect); // Lancia Flameshot con la regione
        // Potresti anche animare i triangoli rimanenti per scomparire o tornare normali
    }
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

    QProcess *flameshotProcess = new QProcess(this);
    connect(flameshotProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "Flameshot terminato con codice:" << exitCode;
        this->close(); // Chiudi l'animatore dopo che Flameshot ha fatto il suo lavoro
        // O QApplication::quit(); per terminare l'intera app se non hai altre finestre
    });
    flameshotProcess->start("flameshot", arguments);

    if (!flameshotProcess->waitForStarted(5000)) { // Attendi fino a 5 secondi per l'avvio
        qWarning() << "Errore: Flameshot non è riuscito ad avviarsi.";
        this->close();
    }
}
