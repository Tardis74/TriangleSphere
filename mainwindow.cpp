#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QWheelEvent>
#include "coordtransform.h"

MainWindow::MainWindow() : updatingTextFromScene(false), blockSceneUpdates(false),
    updatingFromTriangle(false), updatingFromSphere(false),
    lastSpherePoint(0, 0, 0) {
    //qDebug() << "MainWindow constructor started";

    // Setup triangle window
    QWidget* centralWidget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout;

    // Create coordinates text edit with instructions
    coordinatesTextEdit = new QPlainTextEdit;
    coordinatesTextEdit->setMinimumHeight(80);
    coordinatesTextEdit->setStyleSheet("QPlainTextEdit { "
                                       "background-color: #f0f0f0; "
                                       "color: #000000; "
                                       "padding: 5px; "
                                       "border: 1px solid #ccc; "
                                       "font-family: monospace; "
                                       "}");
    coordinatesTextEdit->setPlaceholderText("Coordinates and masses are displayed here");
    coordinatesTextEdit->setReadOnly(true);
    layout->addWidget(coordinatesTextEdit);

    // Create info label
    QLabel* infoLabel = new QLabel("Drag points to modify triangle. Right-click to change mass. Use mouse wheel to zoom.");
    infoLabel->setStyleSheet("QLabel { color: #666; padding: 5px; }");
    layout->addWidget(infoLabel);

    // Create graphics view and scene
    view = new QGraphicsView;
    scene = new TriangleScene;

    view->setScene(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setStyleSheet("QGraphicsView { border: 1px solid #ccc; background-color: white; }");
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    view->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    layout->addWidget(view);

    // Create button to reset view
    QPushButton* resetButton = new QPushButton("Reset View and Zoom");
    resetButton->setStyleSheet("QPushButton { padding: 5px; }");
    layout->addWidget(resetButton);

    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // Setup 3D sphere window
    sphereWidget = new SphereWidget;
    sphereWidget->setWindowTitle("3D Sphere Projection - Drag to rotate, Scroll to zoom, Left-click to set point");
    sphereWidget->resize(600, 600);

    //qDebug() << "MainWindow constructor about to show sphereWidget";
    sphereWidget->show();
    //qDebug() << "MainWindow constructor sphereWidget shown";

    // Connect signals and slots
    connect(scene, &TriangleScene::coordinatesUpdated, this, &MainWindow::updateCoordinatesText);
    // Убираем автоматическое обновление сферы при изменении треугольника
    // connect(scene, &TriangleScene::triangleUpdated, this, &MainWindow::updateSpherePoint);
    connect(sphereWidget, &SphereWidget::spherePointClicked, this, &MainWindow::handleSpherePointClicked);

    // Подключаемся к сигналу окончания перетаскивания
    connect(scene, &TriangleScene::dragFinished, this, &MainWindow::onDragFinished);

    connect(resetButton, &QPushButton::clicked, this, [this]() {
        QList<QPointF> defaultPoints = {
            QPointF(100, 100),
            QPointF(200, 100),
            QPointF(150, 200)
        };
        blockSceneUpdates = true;
        scene->setPoints(defaultPoints);
        blockSceneUpdates = false;

        // Сбрасываем трансформацию и устанавливаем разумный масштаб
        view->resetTransform();
        view->scale(1.0, 1.0); // Устанавливаем единичный масштаб

        // Принудительно обновляем сферу после сброса
        updateSpherePoint();
    });

    // Initial update
    updateCoordinatesText(scene->getPointCoordinates());

    // Delayed call for auto-scaling
    QTimer::singleShot(100, this, &MainWindow::autoScaleView);

    // Delayed call for initial sphere update
    QTimer::singleShot(100, this, &MainWindow::updateSpherePoint);

    //qDebug() << "MainWindow constructor completed";
}

MainWindow::~MainWindow() {
    if (sphereWidget) {
        delete sphereWidget;
    }
}

void MainWindow::updateCoordinatesText(const QString& coords) {
    if (updatingTextFromScene) return;

    updatingTextFromScene = true;
    coordinatesTextEdit->blockSignals(true);
    coordinatesTextEdit->setPlainText(coords);
    coordinatesTextEdit->blockSignals(false);
    updatingTextFromScene = false;
}

void MainWindow::autoScaleView() {
    if (scene && view) {
        QRectF rect = scene->sceneRect();
        if (rect.isValid() && !rect.isNull()) {
            // Ограничиваем максимальный масштаб
            const qreal maxViewSize = 10000.0;
            if (rect.width() > maxViewSize || rect.height() > maxViewSize) {
                // Если сцена слишком большая, не масштабируем автоматически
                return;
            }
            view->fitInView(rect, Qt::KeepAspectRatio);
        }
    }
}

void MainWindow::onDragFinished() {
    // Обновляем сферу только после окончания перетаскивания
    updateSpherePoint();
}

void MainWindow::updateSpherePoint() {
    std::lock_guard<std::mutex> lock(updateMutex);

    if (updatingFromSphere) {
        //qDebug() << "UpdateSpherePoint: updatingFromSphere is true, skipping";
        return;
    }

    //qDebug() << "UpdateSpherePoint: blockSceneUpdates =" << blockSceneUpdates.load();
    if (!sphereWidget || blockSceneUpdates.load()) {
        //qDebug() << "UpdateSpherePoint: early return";
        return;
    }

    updatingFromTriangle = true;

    try {
        // Check if OpenGL context is valid
        if (!sphereWidget->isValid()) {
            qWarning() << "OpenGL context is not valid";
            updatingFromTriangle = false;
            return;
        }

        auto points = scene->getPoints();
        auto masses = scene->getMasses();

        // Validate points before transformation
        for (int i = 0; i < 3; ++i) {
            if (std::isnan(points[i].x()) || std::isinf(points[i].x()) ||
                std::isnan(points[i].y()) || std::isinf(points[i].y())) {
                qWarning() << "Invalid point coordinates in updateSpherePoint:" << points[i];
                updatingFromTriangle = false;
                return;
            }
        }

        QVector3D spherePoint = CoordTransform::transformToSphere(points, masses);
        if (spherePoint.isNull()) {
            qWarning() << "Failed to transform to sphere - invalid sphere point";
            updatingFromTriangle = false;
            return;
        }

        // Check if sphere point is valid
        if (std::isnan(spherePoint.x()) || std::isinf(spherePoint.x()) ||
            std::isnan(spherePoint.y()) || std::isinf(spherePoint.y()) ||
            std::isnan(spherePoint.z()) || std::isinf(spherePoint.z())) {
            qWarning() << "Invalid sphere point coordinates:" << spherePoint;
            updatingFromTriangle = false;
            return;
        }

        // Check if the change is significant enough to update
        QVector3D currentSpherePoint = sphereWidget->getPoint();
        if ((spherePoint - currentSpherePoint).length() < updateThreshold) {
            //qDebug() << "Sphere point change is too small, skipping update";
            updatingFromTriangle = false;
            return;
        }

        sphereWidget->setPoint(spherePoint);
        lastSpherePoint = spherePoint;
    }
    catch (const std::exception& e) {
        qWarning() << "Exception in updateSpherePoint:" << e.what();
    }
    catch (...) {
        qWarning() << "Unknown exception in updateSpherePoint";
    }

    updatingFromTriangle = false;
    //qDebug() << "UpdateSpherePoint: finished";
}

void MainWindow::handleSpherePointClicked(const QVector3D& point) {
    if (updatingFromTriangle) {
        //qDebug() << "HandleSpherePointClicked: updatingFromTriangle is true, skipping";
        return;
    }

    //qDebug() << "HandleSpherePointClicked: blockSceneUpdates =" << blockSceneUpdates.load();
    if (blockSceneUpdates.load()) {
        //qDebug() << "HandleSpherePointClicked: early return";
        return;
    }

    // Check if the change is significant enough to update
    if ((point - lastSpherePoint).length() < updateThreshold) {
        //qDebug() << "Sphere point change is too small, skipping update";
        return;
    }

    updatingFromSphere = true;

    try {
        // Check if sphere point is valid
        if (std::isnan(point.x()) || std::isinf(point.x()) ||
            std::isnan(point.y()) || std::isinf(point.y()) ||
            std::isnan(point.z()) || std::isinf(point.z())) {
            qWarning() << "Invalid sphere point in handleSpherePointClicked:" << point;
            updatingFromSphere = false;
            return;
        }

        auto masses = scene->getMasses();
        auto newPoints = CoordTransform::transformFromSphere(point, masses);

        if (newPoints.size() != 3) {
            qWarning() << "Failed to transform from sphere - invalid points count:" << newPoints.size();
            updatingFromSphere = false;
            return;
        }

        // Validate points before setting them
        for (int i = 0; i < 3; ++i) {
            if (std::isnan(newPoints[i].x()) || std::isinf(newPoints[i].x()) ||
                std::isnan(newPoints[i].y()) || std::isinf(newPoints[i].y())) {
                qWarning() << "Invalid point coordinates from sphere transformation:" << newPoints[i];
                updatingFromSphere = false;
                return;
            }

            // Limit point coordinates to prevent extreme values
            if (std::abs(newPoints[i].x()) > 10000 || std::abs(newPoints[i].y()) > 10000) {
                qWarning() << "Point coordinates out of range:" << newPoints[i];
                updatingFromSphere = false;
                return;
            }
        }

        blockSceneUpdates = true;
        scene->setPoints(newPoints);
        blockSceneUpdates = false;

        autoScaleView();
        lastSpherePoint = point;
    }
    catch (const std::exception& e) {
        qWarning() << "Exception in handleSpherePointClicked:" << e.what();
    }
    catch (...) {
        qWarning() << "Unknown exception in handleSpherePointClicked";
    }

    updatingFromSphere = false;
    //qDebug() << "HandleSpherePointClicked: finished";
}

void MainWindow::wheelEvent(QWheelEvent* event) {
    if (view->underMouse()) {
        // Zoom the triangle view
        double zoomFactor = 1.1;
        if (event->angleDelta().y() < 0) {
            zoomFactor = 1.0 / zoomFactor;
        }
        view->scale(zoomFactor, zoomFactor);
        event->accept();
    } else {
        QMainWindow::wheelEvent(event);
    }
}
