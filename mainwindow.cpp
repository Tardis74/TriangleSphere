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
#include <QGroupBox>
#include <QCheckBox>
#include <cmath>
#include <QRegularExpression>
#include "coordtransform.h"
#include "functioninputdialog.h"

// Простой математический парсер
namespace MathParser {
double parseExpression(const QString& expr);
double evaluateExpression(const QString& expr);

double evaluate(const QString& expression, double t) {
    if (expression.isEmpty()) return 0.0;

    QString expr = expression;
    expr = expr.replace("t", QString::number(t, 'f', 6));
    expr = expr.replace("pi", "3.14159265358979323846");
    expr = expr.replace("e", "2.71828182845904523536");

    // Добавляем поддержку математических констант из ваших уравнений
    expr = expr.replace("0.8660254", "0.8660254037844386"); // sqrt(3)/2

    // Упрощённая замена функций
    expr = expr.replace("sin", "s");
    expr = expr.replace("cos", "c");
    expr = expr.replace("tan", "t");
    expr = expr.replace("exp", "e");
    expr = expr.replace("log", "l");
    expr = expr.replace("sqrt", "q");

    // Удаляем пробелы
    expr = expr.remove(" ");

    return evaluateExpression(expr);
}

double evaluateExpression(const QString& expr) {
    try {
        return parseExpression(expr);
    } catch (const std::exception&) {
        return 0.0;
    }
}

double parseExpression(const QString& expr) {
    int len = expr.length();
    if (len == 0) return 0.0;

    // Обработка скобок
    if (expr[0] == '(' && expr[len-1] == ')') {
        return parseExpression(expr.mid(1, len-2));
    }

    // Поиск операторов с низким приоритетом (+, -)
    int parenCount = 0;
    for (int i = len-1; i >= 0; --i) {
        QChar c = expr[i];
        if (c == ')') parenCount++;
        else if (c == '(') parenCount--;

        if (parenCount == 0 && (c == '+' || c == '-') && i > 0) {
            double left = parseExpression(expr.left(i));
            double right = parseExpression(expr.mid(i+1));
            return (c == '+') ? left + right : left - right;
        }
    }

    // Поиск операторов (*, /)
    parenCount = 0;
    for (int i = len-1; i >= 0; --i) {
        QChar c = expr[i];
        if (c == ')') parenCount++;
        else if (c == '(') parenCount--;

        if (parenCount == 0 && (c == '*' || c == '/') && i > 0) {
            double left = parseExpression(expr.left(i));
            double right = parseExpression(expr.mid(i+1));
            if (c == '/' && right == 0.0) return 0.0; // Защита от деления на ноль
            return (c == '*') ? left * right : left / right;
        }
    }

    // Обработка функций
    if (expr.length() > 1 && (expr[0] == 's' || expr[0] == 'c' || expr[0] == 't' ||
                              expr[0] == 'e' || expr[0] == 'l' || expr[0] == 'q')) {
        double arg = parseExpression(expr.mid(1));
        switch (expr[0].toLatin1()) {
        case 's': return std::sin(arg); // sin
        case 'c': return std::cos(arg); // cos
        case 't': return std::tan(arg); // tan
        case 'e': return std::exp(arg); // exp
        case 'l': return std::log(arg); // log
        case 'q': return std::sqrt(arg); // sqrt
        }
    }

    // Числа
    bool ok;
    double result = expr.toDouble(&ok);
    if (ok) return result;

    return 0.0;
}
}

MainWindow::MainWindow() :
    blockSceneUpdates(false),
    lastSpherePoint(0, 0, 0),
    isAnimationMode(false),
    currentTime(0.0),
    timeStep(0.1),
    maxTime(10.0)
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

        // В конструкторе MainWindow заменяем существующий код для чекбокса:
        QHBoxLayout* modeSwitchLayout = new QHBoxLayout;

        // Создаем контейнер для чекбокса и подписи
        QWidget* animationModeContainer = new QWidget;
        QHBoxLayout* containerLayout = new QHBoxLayout(animationModeContainer);
        containerLayout->setContentsMargins(0, 0, 0, 0);

        QCheckBox* animationModeCheckbox = new QCheckBox("Animation Mode");
        animationModeCheckbox->setChecked(false);
        animationModeCheckbox->setStyleSheet("QCheckBox { font-weight: bold; color: #333; padding: 5px; }"
                                             "QCheckBox::indicator { width: 15px; height: 15px; }"
                                             "QCheckBox::indicator:checked { background-color: #4CAF50; }");

        // Добавляем подпись
        QLabel* animationModeLabel = new QLabel("(включите для задания движения точек по функциям)");
        animationModeLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");

        containerLayout->addWidget(animationModeCheckbox);
        containerLayout->addWidget(animationModeLabel);
        containerLayout->addStretch();

        modeSwitchLayout->addWidget(animationModeContainer);
        leftLayout->addLayout(modeSwitchLayout);

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

        // Панель анимации
        QFrame* animationFrame = new QFrame;
        animationFrame->setFrameStyle(QFrame::Box);
        animationFrame->setLineWidth(1);
        QVBoxLayout* animationFrameLayout = new QVBoxLayout(animationFrame);

        QLabel* animationTitle = new QLabel("Анимация");
        animationTitle->setStyleSheet("QLabel { font-weight: bold; color: black; }");
        animationFrameLayout->addWidget(animationTitle);

        // Кнопки анимации
        QHBoxLayout* animationButtonLayout = new QHBoxLayout;
        animationModeButton = new QPushButton("Set Functions");
        animationModeButton->setFixedHeight(35);
        animationModeButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");
        animationModeButton->setEnabled(false); // Изначально выключена

        animationStartButton = new QPushButton("Start");
        animationStartButton->setFixedHeight(35);
        animationStartButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");
        animationStartButton->setEnabled(false);

        animationPauseButton = new QPushButton("Pause");
        animationPauseButton->setFixedHeight(35);
        animationPauseButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");
        animationPauseButton->setEnabled(false);

        animationResetButton = new QPushButton("Reset");
        animationResetButton->setFixedHeight(35);
        animationResetButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");
        animationResetButton->setEnabled(false);

        animationButtonLayout->addWidget(animationModeButton);
        animationButtonLayout->addWidget(animationStartButton);
        animationButtonLayout->addWidget(animationPauseButton);
        animationButtonLayout->addWidget(animationResetButton);
        animationButtonLayout->addStretch();

        animationFrameLayout->addLayout(animationButtonLayout);

        // Параметры анимации
        QHBoxLayout* animationParamsLayout = new QHBoxLayout;

        QLabel* maxTimeLabel = new QLabel("Max Time:");
        maxTimeEdit = new QLineEdit("20.0");
        // Используем QDoubleValidator с локалью C (всегда использует точку)
        QDoubleValidator* maxTimeValidator = new QDoubleValidator(1.0, 1000.0, 3, this);
        maxTimeValidator->setLocale(QLocale::C); // Принудительно используем точку как разделитель
        maxTimeEdit->setValidator(maxTimeValidator);
        maxTimeEdit->setMaximumWidth(60);
        maxTimeEdit->setEnabled(true);

        QLabel* speedLabel = new QLabel("Speed:");
        speedEdit = new QLineEdit("1.0");
        QDoubleValidator* speedValidator = new QDoubleValidator(0.01, 100.0, 3, this);
        speedValidator->setLocale(QLocale::C); // Принудительно используем точку как разделитель
        speedEdit->setValidator(speedValidator);
        speedEdit->setMaximumWidth(60);
        speedEdit->setEnabled(true);

        showTrajectoryCheckbox = new QCheckBox("Show Trajectory");
        showTrajectoryCheckbox->setEnabled(true);

        animationParamsLayout->addWidget(maxTimeLabel);
        animationParamsLayout->addWidget(maxTimeEdit);
        animationParamsLayout->addWidget(speedLabel);
        animationParamsLayout->addWidget(speedEdit);
        animationParamsLayout->addWidget(showTrajectoryCheckbox);
        animationParamsLayout->addStretch();

        animationFrameLayout->addLayout(animationParamsLayout);

        // Слайдер времени
        QHBoxLayout* timeLayout = new QHBoxLayout;
        timeLayout->addWidget(new QLabel("Time:"));

        timeSlider = new QSlider(Qt::Horizontal);
        timeSlider->setRange(0, 100);
        timeSlider->setValue(0);
        timeSlider->setEnabled(false);
        timeLayout->addWidget(timeSlider);

        timeLabel = new QLabel("t = 0.00");
        timeLabel->setStyleSheet("QLabel { font-family: monospace; }");
        timeLabel->setMinimumWidth(60);
        timeLayout->addWidget(timeLabel);

        animationFrameLayout->addLayout(timeLayout);
        leftLayout->addWidget(animationFrame);

        QPushButton* stopDrawingButton = new QPushButton("Stop Drawing");
        stopDrawingButton->setFixedHeight(35);
        stopDrawingButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        // Добавьте эту кнопку в animationButtonLayout:
        animationButtonLayout->addWidget(stopDrawingButton);

        // Подключите кнопку:
        connect(stopDrawingButton, &QPushButton::clicked, this, [this, stopDrawingButton]() {
            bool drawingEnabled = sphereWidget->isDrawingEnabled();
            sphereWidget->setDrawingEnabled(!drawingEnabled);

            if (drawingEnabled) {
                stopDrawingButton->setText("Resume Drawing");
            } else {
                stopDrawingButton->setText("Stop Drawing");
            }
        });

        connect(maxTimeEdit, &QLineEdit::editingFinished, this, &MainWindow::fixMaxTimeInput);
        connect(speedEdit, &QLineEdit::editingFinished, this, &MainWindow::fixSpeedInput);

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
        resetButton->setFixedHeight(35);
        resetButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        // Кнопка переключения режимов сферы
        toggleModeButton = new QPushButton("Switch to Point Move");
        toggleModeButton->setFixedHeight(35);
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
        setWindowTitle("Triangle-Sphere Projection System with Animation");
        resize(1200, 700);

        // Создаем таймер анимации
        animationTimer = new QTimer(this);
        animationTimer->setInterval(50); // 20 FPS

        // Подключаем сигналы и слоты
        connect(scene, &TriangleScene::dragFinished, this, &MainWindow::onDragFinished);
        connect(scene, &TriangleScene::triangleUpdated, this, &MainWindow::updatePointCoordinates);
        connect(scene, &TriangleScene::pointPositionChanging, this, &MainWindow::updateSpherePoint); // НОВОЕ
        connect(sphereWidget, &SphereWidget::spherePointClicked, this, &MainWindow::handleSpherePointClicked);
        connect(scene, &TriangleScene::triangleUpdated, this, &MainWindow::updateSpherePoint);

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

        // Подключаем кнопку переключения режимов сферы
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

        // Подключаем анимацию
        connect(animationModeButton, &QPushButton::clicked, this, &MainWindow::onAnimationModeClicked);
        connect(animationStartButton, &QPushButton::clicked, this, &MainWindow::onAnimationStart);
        connect(animationPauseButton, &QPushButton::clicked, this, &MainWindow::onAnimationPause);
        connect(animationResetButton, &QPushButton::clicked, this, &MainWindow::onAnimationReset);
        connect(timeSlider, &QSlider::valueChanged, this, &MainWindow::onTimeSliderChanged);
        connect(animationTimer, &QTimer::timeout, this, &MainWindow::updateAnimation);

        // Подключаем чекбокс траектории к sphereWidget
        connect(showTrajectoryCheckbox, &QCheckBox::toggled, sphereWidget, &SphereWidget::setShowTrajectory);

        // ПОДКЛЮЧАЕМ ЧЕКБОКС ANIMATION MODE
        connect(animationModeCheckbox, &QCheckBox::toggled, this, [this](bool checked) {
            isAnimationMode = checked;

            // Включаем/выключаем элементы управления анимацией
            animationModeButton->setEnabled(checked);
            animationStartButton->setEnabled(checked);
            animationResetButton->setEnabled(checked);
            timeSlider->setEnabled(checked);

            // ВКЛЮЧАЕМ ПОЛЯ ВВОДА ДАЖЕ В РЕЖИМЕ АНИМАЦИИ
            maxTimeEdit->setEnabled(true);  // Всегда доступно
            speedEdit->setEnabled(true);    // Всегда доступно

            showTrajectoryCheckbox->setEnabled(true);

            if (checked) {
                // Включаем режим анимации
                if (x1Func.isEmpty()) {
                    // Если функции не установлены, устанавливаем значения по умолчанию
                    x1Func = "50 + 20*cos(t)";
                    y1Func = "50 + 20*sin(t)";
                    x2Func = "100 + 15*cos(2*t)";
                    y2Func = "50 + 15*sin(2*t)";
                    x3Func = "75 + 25*cos(0.5*t)";
                    y3Func = "100 + 25*sin(0.5*t)";
                }
                currentTime = 0.0;
                timeSlider->setValue(0);
                evaluateFunctions(currentTime);
            } else {
                // Выключаем режим анимации
                animationTimer->stop();
                animationStartButton->setEnabled(false);
                animationPauseButton->setEnabled(false);
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
    if (updatingFromSphere || blockSceneUpdates.load() || !sphereWidget || !scene) {
        return;
    }

    updatingFromTriangle = true;

    try {
        auto points = scene->getPoints();
        auto masses = scene->getMasses();

        if (points.size() == 3 && masses.size() == 3) {
            QVector3D spherePoint = CoordTransform::transformToSphere(points, masses);
            if (!spherePoint.isNull()) {
                sphereWidget->setPoint(spherePoint);
                lastSpherePoint = spherePoint;

                if (sphereCoordsLabel) {
                    sphereCoordsLabel->setText(QString("Sphere point: (%1, %2, %3)")
                                                   .arg(spherePoint.x(), 0, 'f', 3)
                                                   .arg(spherePoint.y(), 0, 'f', 3)
                                                   .arg(spherePoint.z(), 0, 'f', 3));
                }
            }
        }
    }
    catch (const std::exception& e) {
        qWarning() << "Exception in updateSpherePoint:" << e.what();
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

void MainWindow::onAnimationModeClicked()
{
    FunctionInputDialog dialog;
    QStringList currentFunctions = {x1Func, y1Func, x2Func, y2Func, x3Func, y3Func};
    if (!currentFunctions[0].isEmpty()) {
        dialog.setFunctions(currentFunctions);
    }

    if (dialog.exec() == QDialog::Accepted) {
        x1Func = dialog.getX1();
        y1Func = dialog.getY1();
        x2Func = dialog.getX2();
        y2Func = dialog.getY2();
        x3Func = dialog.getX3();
        y3Func = dialog.getY3();

        animationStartButton->setEnabled(true);
        animationResetButton->setEnabled(true);
        timeSlider->setEnabled(true);

        // Сбрасываем время и вычисляем начальное положение
        currentTime = 0.0;
        timeSlider->setValue(0);
        evaluateFunctions(currentTime);

        QMessageBox::information(this, "Animation Mode",
                                 "Functions set! Use the time slider or Start button to begin animation.");
    }
}

void MainWindow::onAnimationStart()
{
    animationTimer->start();
    animationStartButton->setEnabled(false);
    animationPauseButton->setEnabled(true);
}

void MainWindow::onAnimationPause()
{
    animationTimer->stop();
    animationStartButton->setEnabled(true);
    animationPauseButton->setEnabled(false);
}

void MainWindow::onAnimationReset()
{
    animationTimer->stop();
    currentTime = 0.0;
    timeSlider->setValue(0);
    evaluateFunctions(currentTime);
    animationStartButton->setEnabled(true);
    animationPauseButton->setEnabled(false);

    // Очищаем траекторию при сбросе
    sphereWidget->clearTrajectory();
}

void MainWindow::onTimeSliderChanged(int value)
{
    maxTime = maxTimeEdit->text().toDouble();
    currentTime = (value / 100.0) * maxTime;
    evaluateFunctions(currentTime);
    updateTimeLabel();
}

void MainWindow::updateAnimation()
{
    maxTime = maxTimeEdit->text().toDouble();
    double speedFactor = speedEdit->text().toDouble();

    double previousTime = currentTime;
    currentTime += timeStep * speedFactor;

    // Если время перескочило с максимума на минимум, разрываем траекторию
    if (previousTime > maxTime * 0.9 && currentTime < maxTime * 0.1) {
        sphereWidget->breakTrajectory();
    }

    if (currentTime > maxTime) {
        currentTime = 0.0;
    }

    int sliderValue = static_cast<int>((currentTime / maxTime) * 100);
    timeSlider->setValue(sliderValue);

    evaluateFunctions(currentTime);
    updateTimeLabel();
}

void MainWindow::updateTimeLabel()
{
    timeLabel->setText(QString("t = %1").arg(currentTime, 0, 'f', 2));
}

double MainWindow::evaluateExpression(const QString& expression, double t) {
    if (expression.isEmpty()) return 0.0;

    try {
        return MathParser::evaluate(expression, t);
    } catch (const std::exception& e) {
        qWarning() << "Expression evaluation error:" << expression << "->" << e.what();
        return 0.0;
    }
}

void MainWindow::evaluateFunctions(double t)
{
    if (!isAnimationMode) return;

    double x1 = evaluateExpression(x1Func, t);
    double y1 = evaluateExpression(y1Func, t);
    double x2 = evaluateExpression(x2Func, t);
    double y2 = evaluateExpression(y2Func, t);
    double x3 = evaluateExpression(x3Func, t);
    double y3 = evaluateExpression(y3Func, t);

    QList<QPointF> points = {
        QPointF(x1, y1),
        QPointF(x2, y2),
        QPointF(x3, y3)
    };

    blockSceneUpdates = true;
    scene->setPoints(points);
    blockSceneUpdates = false;

    // ВЫЧИСЛЯЕМ ТОЧКУ НА СФЕРЕ И ДОБАВЛЯЕМ В ТРАЕКТОРИЮ
    auto masses = scene->getMasses();
    QVector3D spherePoint = CoordTransform::transformToSphere(points, masses);

    if (!spherePoint.isNull()) {
        // Обновляем точку на сфере
        sphereWidget->setPoint(spherePoint);

        // Добавляем в траекторию если включено отображение
        if (showTrajectoryCheckbox->isChecked()) {
            sphereWidget->addToTrajectory(spherePoint);
        }

        // Обновляем label
        if (sphereCoordsLabel) {
            sphereCoordsLabel->setText(QString("Sphere point: (%1, %2, %3)")
                                           .arg(spherePoint.x(), 0, 'f', 3)
                                           .arg(spherePoint.y(), 0, 'f', 3)
                                           .arg(spherePoint.z(), 0, 'f', 3));
        }
    }

    updatePointCoordinates();
}

void MainWindow::fixMaxTimeInput()
{
    QString text = maxTimeEdit->text();
    // Заменяем запятые на точки
    text = text.replace(',', '.');
    // Удаляем лишние точки
    int dotCount = text.count('.');
    if (dotCount > 1) {
        int firstDot = text.indexOf('.');
        text = text.left(firstDot + 1) + text.mid(firstDot + 1).remove('.');
    }
    maxTimeEdit->setText(text);
}

void MainWindow::fixSpeedInput()
{
    QString text = speedEdit->text();
    // Заменяем запятые на точки
    text = text.replace(',', '.');
    // Удаляем лишние точки
    int dotCount = text.count('.');
    if (dotCount > 1) {
        int firstDot = text.indexOf('.');
        text = text.left(firstDot + 1) + text.mid(firstDot + 1).remove('.');
    }
    speedEdit->setText(text);
}
