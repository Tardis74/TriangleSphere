#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QLineEdit>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QFrame>
#include "coordtransform.h"

MainWindow::MainWindow() :
    blockSceneUpdates(false),
    lastSpherePoint(0, 0, 0)
{
    qDebug() << "MainWindow constructor started";

    try {
        // Создаем главный splitter
        mainSplitter = new QSplitter(Qt::Horizontal, this);

        // Левая панель - треугольник
        QWidget* leftPanel = new QWidget;
        QVBoxLayout* leftLayout = new QVBoxLayout;

        // Создаем метку для координат сферы
        sphereCoordsLabel = new QLabel("Sphere point: (0.000, 0.000, 0.000)");
        sphereCoordsLabel->setStyleSheet("QLabel { color: #333; padding: 8px; background-color: #f0f0f0; border: 1px solid #ccc; font-weight: bold; }");
        leftLayout->addWidget(sphereCoordsLabel);

        // Создаем info label
        QLabel* infoLabel = new QLabel("Drag points to modify triangle. Right-click to change mass. Use +/- buttons to zoom. Drag background to move entire triangle. Use 'Switch Mode' button to toggle between sphere rotation and point movement modes.");
        infoLabel->setStyleSheet("QLabel { color: #666; padding: 8px; background-color: #f8f8f8; border: 1px solid #ddd; }");
        infoLabel->setWordWrap(true);
        leftLayout->addWidget(infoLabel);

        // Создаем graphics view и scene
        view = new QGraphicsView;
        scene = new TriangleScene(this);

        qDebug() << "Scene and view created";

        view->setScene(scene);
        view->setRenderHint(QPainter::Antialiasing);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        view->setStyleSheet("QGraphicsView { border: 2px solid #aaa; background-color: white; }");
        view->setDragMode(QGraphicsView::RubberBandDrag);

        leftLayout->addWidget(view);

        // Фрейм для координат точек (только чтение)
        QFrame* coordsFrame = new QFrame;
        coordsFrame->setFrameStyle(QFrame::Box);
        coordsFrame->setLineWidth(1);
        QVBoxLayout* coordsFrameLayout = new QVBoxLayout(coordsFrame);

        QLabel* coordsTitle = new QLabel("Координаты точек");
        coordsTitle->setStyleSheet("QLabel { font-weight: bold; color: black; }");
        coordsFrameLayout->addWidget(coordsTitle);

        coordsLabel = new QLabel();
        coordsLabel->setStyleSheet("QLabel { color: #333; padding: 5px; background-color: #f8f8f8; border: 1px solid #ddd; font-family: monospace; }");
        coordsLabel->setTextFormat(Qt::PlainText);
        coordsLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        coordsFrameLayout->addWidget(coordsLabel);

        leftLayout->addWidget(coordsFrame);

        // Панель для ввода масс
        QFrame* massFrame = new QFrame;
        massFrame->setFrameStyle(QFrame::Box);
        massFrame->setLineWidth(1);
        QVBoxLayout* massFrameLayout = new QVBoxLayout(massFrame);

        QLabel* massTitle = new QLabel("Массы");
        massTitle->setStyleSheet("QLabel { font-weight: bold; color: black; }");
        massFrameLayout->addWidget(massTitle);

        QHBoxLayout* massLayout = new QHBoxLayout;

        mass1Edit = new QLineEdit("1.0");
        mass2Edit = new QLineEdit("1.0");
        mass3Edit = new QLineEdit("1.0");

        // Устанавливаем валидаторы для ввода только чисел
        QDoubleValidator* validator = new QDoubleValidator(0.001, 1000.0, 3, this);
        mass1Edit->setValidator(validator);
        mass2Edit->setValidator(validator);
        mass3Edit->setValidator(validator);

        mass1Edit->setMaximumWidth(60);
        mass2Edit->setMaximumWidth(60);
        mass3Edit->setMaximumWidth(60);

        setMassesButton = new QPushButton("Set Masses");
        setMassesButton->setStyleSheet("QPushButton { padding: 5px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        massLayout->addWidget(new QLabel("P1:"));
        massLayout->addWidget(mass1Edit);
        massLayout->addWidget(new QLabel("P2:"));
        massLayout->addWidget(mass2Edit);
        massLayout->addWidget(new QLabel("P3:"));
        massLayout->addWidget(mass3Edit);
        massLayout->addWidget(setMassesButton);
        massLayout->addStretch();

        massFrameLayout->addLayout(massLayout);
        leftLayout->addWidget(massFrame);

        // Создаем панель управления с кнопками масштабирования
        QHBoxLayout* controlLayout = new QHBoxLayout;

        // Кнопка уменьшения масштаба
        zoomOutButton = new QPushButton("-");
        zoomOutButton->setFixedSize(40, 35);
        zoomOutButton->setStyleSheet("QPushButton { font-size: 18px; font-weight: bold; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        // Кнопка увеличения масштаба
        zoomInButton = new QPushButton("+");
        zoomInButton->setFixedSize(40, 35);
        zoomInButton->setStyleSheet("QPushButton { font-size: 18px; font-weight: bold; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        // Кнопка сброса
        resetButton = new QPushButton("Reset View");
        resetButton->setFixedHeight(35); // Устанавливаем такую же высоту, как у других кнопок
        resetButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        // Кнопка переключения режимов сферы - одинакового размера с resetButton
        toggleModeButton = new QPushButton("Switch to Point Move");
        toggleModeButton->setFixedHeight(35); // Такая же высота, как у resetButton
        toggleModeButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        // Добавляем кнопки в layout
        controlLayout->addWidget(zoomOutButton);
        controlLayout->addWidget(zoomInButton);
        controlLayout->addWidget(resetButton);
        controlLayout->addWidget(toggleModeButton);
        controlLayout->addStretch();

        leftLayout->addLayout(controlLayout);

        leftPanel->setLayout(leftLayout);

        // Правая панель - сфера
        sphereWidget = new SphereWidget;
        sphereWidget->setMinimumSize(500, 500);

        qDebug() << "SphereWidget created";

        // Добавляем панели в splitter
        mainSplitter->addWidget(leftPanel);
        mainSplitter->addWidget(sphereWidget);

        // Устанавливаем пропорции (40% треугольник, 60% сфера)
        mainSplitter->setStretchFactor(0, 4);
        mainSplitter->setStretchFactor(1, 6);

        setCentralWidget(mainSplitter);
        setWindowTitle("Triangle-Sphere Projection System");
        resize(1200, 700);

        // Подключаем сигналы и слоты
        connect(scene, &TriangleScene::dragFinished, this, &MainWindow::onDragFinished);
        connect(scene, &TriangleScene::triangleUpdated, this, &MainWindow::updatePointCoordinates);
        connect(sphereWidget, &SphereWidget::spherePointClicked, this, &MainWindow::handleSpherePointClicked);
        connect(scene, &TriangleScene::pointPositionChanging, this, &MainWindow::updateSpherePoint);

        // Подключаем сигналы для изменения курсора
        connect(scene, &TriangleScene::sceneDragStarted, this, [this]() {
            if (view) {
                view->setCursor(Qt::ClosedHandCursor);
            }
        });
        connect(scene, &TriangleScene::sceneDragFinished, this, [this]() {
            if (view) {
                view->setCursor(Qt::ArrowCursor);
            }
        });

        // Подключаем кнопки масштабирования
        connect(zoomInButton, &QPushButton::clicked, this, &MainWindow::zoomIn);
        connect(zoomOutButton, &QPushButton::clicked, this, &MainWindow::zoomOut);

        // Подключаем кнопку установки масс
        connect(setMassesButton, &QPushButton::clicked, this, &MainWindow::setMasses);

        // Подключаем кнопку переключения режимов
        connect(toggleModeButton, &QPushButton::clicked, this, [this]() {
            bool currentMode = sphereWidget->getRotationMode();
            sphereWidget->setRotationMode(!currentMode);

            if (!currentMode) {
                toggleModeButton->setText("Switch to Point Move");
                if (view) view->setCursor(Qt::ArrowCursor);
            } else {
                toggleModeButton->setText("Switch to Sphere Rotate");
                if (view) view->setCursor(Qt::PointingHandCursor);
            }
        });

        connect(resetButton, &QPushButton::clicked, this, [this]() {
            QList<QPointF> defaultPoints = {
                QPointF(50, 50),
                QPointF(100, 50),
                QPointF(75, 100)
            };
            blockSceneUpdates = true;
            scene->setPoints(defaultPoints);
            blockSceneUpdates = false;

            // Сбрасываем массы в полях ввода
            mass1Edit->setText("1.0");
            mass2Edit->setText("1.0");
            mass3Edit->setText("1.0");

            view->resetTransform();
            view->scale(1.0, 1.0);
            view->centerOn(75, 75);

            updateSpherePoint();
            updatePointCoordinates();
        });

        // Центрируем вид на треугольник
        QTimer::singleShot(100, this, [this]() {
            if (view) {
                view->resetTransform();
                view->scale(1.0, 1.0);
                view->centerOn(75, 75);
            }
        });

        // Обновляем сферу и координаты
        QTimer::singleShot(100, this, &MainWindow::updateSpherePoint);
        QTimer::singleShot(100, this, &MainWindow::updatePointCoordinates);

        qDebug() << "MainWindow constructor completed successfully";
    }
    catch (const std::exception& e) {
        qCritical() << "Exception in MainWindow constructor: " << e.what();
    }
    catch (...) {
        qCritical() << "Unknown exception in MainWindow constructor";
    }
}

MainWindow::~MainWindow()
{
    if (sphereWidget) {
        delete sphereWidget;
    }
}

void MainWindow::updateSpherePoint()
{
    if (updatingFromSphere) {
        return;
    }

    if (!sphereWidget || !scene) {
        return;
    }

    updatingFromTriangle = true;

    try {
        if (!sphereWidget->isValid()) {
            qWarning() << "OpenGL context is not valid";
            updatingFromTriangle = false;
            return;
        }

        auto points = scene->getPoints();
        auto masses = scene->getMasses();

        if (points.size() != 3 || masses.size() != 3) {
            qWarning() << "Invalid points or masses count";
            updatingFromTriangle = false;
            return;
        }

        sphereWidget->setMasses(masses);

        QVector3D spherePoint = CoordTransform::transformToSphere(points, masses);
        if (spherePoint.isNull()) {
            qWarning() << "Failed to transform to sphere";
            updatingFromTriangle = false;
            return;
        }

        QVector3D currentSpherePoint = sphereWidget->getPoint();
        if ((spherePoint - currentSpherePoint).length() < updateThreshold) {
            updatingFromTriangle = false;
            return;
        }

        sphereWidget->setPoint(spherePoint);

        lastSpherePoint = spherePoint;

        if (sphereCoordsLabel) {
            sphereCoordsLabel->setText(QString("Sphere point: (%1, %2, %3)")
                                           .arg(spherePoint.x(), 0, 'f', 3)
                                           .arg(spherePoint.y(), 0, 'f', 3)
                                           .arg(spherePoint.z(), 0, 'f', 3));
        }
    }
    catch (const std::exception& e) {
        qWarning() << "Exception in updateSpherePoint:" << e.what();
    }
    catch (...) {
        qWarning() << "Unknown exception in updateSpherePoint";
    }

    updatingFromTriangle = false;
}

void MainWindow::updatePointCoordinates()
{
    if (!scene) return;

    auto points = scene->getPoints();
    if (points.size() != 3) return;

    // Форматируем текст в требуемом формате
    QString text = QString("P1-> x: %1 y: %2\nP2-> x: %3 y: %4\nP3-> x: %5 y: %6")
                       .arg(points[0].x(), 0, 'f', 1)
                       .arg(points[0].y(), 0, 'f', 1)
                       .arg(points[1].x(), 0, 'f', 1)
                       .arg(points[1].y(), 0, 'f', 1)
                       .arg(points[2].x(), 0, 'f', 1)
                       .arg(points[2].y(), 0, 'f', 1);

    coordsLabel->setText(text);
}

void MainWindow::handleSpherePointClicked(const QVector3D& point)
{
    if (updatingFromTriangle || blockSceneUpdates.load()) {
        return;
    }

    if ((point - lastSpherePoint).length() < updateThreshold) {
        return;
    }

    updatingFromSphere = true;

    try {
        if (std::isnan(point.x()) || std::isinf(point.x()) ||
            std::isnan(point.y()) || std::isinf(point.y()) ||
            std::isnan(point.z()) || std::isinf(point.z())) {
            qWarning() << "Invalid sphere point in handleSpherePointClicked:" << point;
            updatingFromSphere = false;
            return;
        }

        auto masses = scene->getMasses();
        auto newPoints = CoordTransform::transformFromSphere(point, masses, 100.0);

        if (newPoints.size() != 3) {
            qWarning() << "Failed to transform from sphere - invalid points count:" << newPoints.size();
            updatingFromSphere = false;
            return;
        }

        blockSceneUpdates = true;
        scene->setPoints(newPoints);
        blockSceneUpdates = false;

        lastSpherePoint = point;

        if (sphereCoordsLabel) {
            sphereCoordsLabel->setText(QString("Sphere point: (%1, %2, %3)")
                                           .arg(point.x(), 0, 'f', 3)
                                           .arg(point.y(), 0, 'f', 3)
                                           .arg(point.z(), 0, 'f', 3));
        }

        updatePointCoordinates();
    }
    catch (const std::exception& e) {
        qWarning() << "Exception in handleSpherePointClicked:" << e.what();
    }
    catch (...) {
        qWarning() << "Unknown exception in handleSpherePointClicked";
    }

    updatingFromSphere = false;
}

void MainWindow::onDragFinished()
{
    updateSpherePoint();
    updatePointCoordinates();
}

void MainWindow::zoomIn()
{
    if (view) {
        QTransform transform = view->transform();
        double currentScale = transform.m11();

        if (currentScale < 5.0) {
            view->scale(1.2, 1.2);
        }
    }
}

void MainWindow::zoomOut()
{
    if (view) {
        QTransform transform = view->transform();
        double currentScale = transform.m11();

        if (currentScale > 0.2) {
            view->scale(1.0 / 1.2, 1.0 / 1.2);
        }
    }
}

void MainWindow::setMasses()
{
    // Получаем значения из полей ввода
    bool ok1, ok2, ok3;

    // Заменяем запятые на точки для корректного преобразования
    QString mass1Text = mass1Edit->text().replace(',', '.');
    QString mass2Text = mass2Edit->text().replace(',', '.');
    QString mass3Text = mass3Edit->text().replace(',', '.');

    double mass1 = mass1Text.toDouble(&ok1);
    double mass2 = mass2Text.toDouble(&ok2);
    double mass3 = mass3Text.toDouble(&ok3);

    if (!ok1 || !ok2 || !ok3) {
        QMessageBox::warning(this, "Invalid Input", "Please enter valid numbers for all masses.");
        return;
    }

    if (mass1 <= 0 || mass2 <= 0 || mass3 <= 0) {
        QMessageBox::warning(this, "Invalid Mass", "Masses must be positive numbers.");
        return;
    }

    // Устанавливаем массы
    QList<double> masses = {mass1, mass2, mass3};
    scene->setMasses(masses);

    // Обновляем сферу и информацию о точках
    updateSpherePoint();
    updatePointCoordinates();
}

void MainWindow::autoScaleView()
{
    if (view) {
        view->resetTransform();
        view->scale(1.0, 1.0);
        view->centerOn(75, 75);
    }
}
