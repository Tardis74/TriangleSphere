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
        // Создаем главный splitter (горизонтальный)
        mainSplitter = new QSplitter(Qt::Horizontal, this);

        // ЛЕВАЯ ЧАСТЬ - треугольник и исходная плоскость
        QWidget* leftContainer = new QWidget;
        QVBoxLayout* leftMainLayout = new QVBoxLayout(leftContainer);

        // Создаем вертикальный сплиттер для левой части
        leftSplitter = new QSplitter(Qt::Vertical);

        // ВЕРХНЯЯ ЛЕВАЯ ЧАСТЬ - треугольник
        QWidget* triangleContainer = new QWidget;
        QVBoxLayout* triangleLayout = new QVBoxLayout(triangleContainer);

        // Создаем метку для координат сферы
        sphereCoordsLabel = new QLabel("Sphere point: (0.000, 0.000, 0.000)");
        sphereCoordsLabel->setStyleSheet("QLabel { color: #333; padding: 8px; background-color: #f0f0f0; border: 1px solid #ccc; font-weight: bold; }");
        triangleLayout->addWidget(sphereCoordsLabel);

        // Информация о комплексных координатах и радиусе
        QHBoxLayout* infoLayout = new QHBoxLayout;
        complexCoordsLabel = new QLabel("Complex: (0.000, 0.000)");
        complexCoordsLabel->setStyleSheet("QLabel { color: #333; padding: 8px; background-color: #f0f0f0; border: 1px solid #ccc; font-weight: bold; }");

        radiusLabel = new QLabel("Radius: 1.000");
        radiusLabel->setStyleSheet("QLabel { color: #333; padding: 8px; background-color: #f0f0f0; border: 1px solid #ccc; font-weight: bold; }");

        infoLayout->addWidget(complexCoordsLabel);
        infoLayout->addWidget(radiusLabel);
        triangleLayout->addLayout(infoLayout);

        // Кнопка переключения режимов сферы
        QHBoxLayout* modeSwitchLayout = new QHBoxLayout;
        toggleModeButton = new QPushButton("Switch to Point Move");
        toggleModeButton->setFixedHeight(35);
        toggleModeButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");
        modeSwitchLayout->addWidget(toggleModeButton);
        modeSwitchLayout->addStretch();
        triangleLayout->addLayout(modeSwitchLayout);

        // Контейнер для чекбокса анимации
        QWidget* animationModeContainer = new QWidget;
        QHBoxLayout* containerLayout = new QHBoxLayout(animationModeContainer);
        containerLayout->setContentsMargins(0, 0, 0, 0);

        QCheckBox* animationModeCheckbox = new QCheckBox("Animation Mode");
        animationModeCheckbox->setChecked(false);
        animationModeCheckbox->setStyleSheet("QCheckBox { font-weight: bold; padding: 5px; }"
                                             "QCheckBox::indicator { width: 15px; height: 15px; }"
                                             "QCheckBox::indicator:checked { background-color: #4CAF50; }");

        QLabel* animationModeLabel = new QLabel("(включите для задания движения точек по функциям)");
        animationModeLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");

        containerLayout->addWidget(animationModeCheckbox);
        containerLayout->addWidget(animationModeLabel);
        containerLayout->addStretch();

        triangleLayout->addWidget(animationModeContainer);

        // Создаем graphics view и scene для треугольника
        view = new QGraphicsView;
        scene = new TriangleScene(this);

        qDebug() << "Scene and view created";

        view->setScene(scene);
        view->setRenderHint(QPainter::Antialiasing);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        view->setStyleSheet("QGraphicsView { border: 2px solid #aaa; background-color: white; }");
        view->setDragMode(QGraphicsView::RubberBandDrag);

        triangleLayout->addWidget(view);

        // Панель управления масштабированием треугольника
        QHBoxLayout* controlLayout = new QHBoxLayout;

        zoomOutButton = new QPushButton("-");
        zoomOutButton->setFixedSize(40, 35);
        zoomOutButton->setStyleSheet("QPushButton { font-size: 18px; font-weight: bold; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        zoomInButton = new QPushButton("+");
        zoomInButton->setFixedSize(40, 35);
        zoomInButton->setStyleSheet("QPushButton { font-size: 18px; font-weight: bold; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        resetButton = new QPushButton("Reset View");
        resetButton->setFixedHeight(35);
        resetButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        QPushButton* autoScaleButton = new QPushButton("Auto Scale Triangle");
        autoScaleButton->setFixedHeight(35);
        autoScaleButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        controlLayout->addWidget(zoomOutButton);
        controlLayout->addWidget(zoomInButton);
        controlLayout->addWidget(resetButton);
        controlLayout->addWidget(autoScaleButton);
        controlLayout->addStretch();

        triangleLayout->addLayout(controlLayout);

        // Фрейм для координат точек
        QFrame* coordsFrame = new QFrame;
        coordsFrame->setFrameStyle(QFrame::Box);
        coordsFrame->setLineWidth(1);
        QVBoxLayout* coordsFrameLayout = new QVBoxLayout(coordsFrame);

        QLabel* coordsTitle = new QLabel("Point's coordinates");
        coordsTitle->setStyleSheet("QLabel { font-weight: bold; }");
        coordsFrameLayout->addWidget(coordsTitle);

        coordsLabel = new QLabel();
        coordsLabel->setStyleSheet("QLabel { color: #333; padding: 5px; background-color: #f8f8f8; border: 1px solid #ddd; font-family: monospace; }");
        coordsLabel->setTextFormat(Qt::PlainText);
        coordsLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        coordsFrameLayout->addWidget(coordsLabel);

        triangleLayout->addWidget(coordsFrame);

        // Панель для ввода масс
        QFrame* massFrame = new QFrame;
        massFrame->setFrameStyle(QFrame::Box);
        massFrame->setLineWidth(1);
        QVBoxLayout* massFrameLayout = new QVBoxLayout(massFrame);

        QLabel* massTitle = new QLabel("Masses");
        massTitle->setStyleSheet("QLabel { font-weight: bold; }");
        massFrameLayout->addWidget(massTitle);

        QHBoxLayout* massLayout = new QHBoxLayout;

        mass1Edit = new QLineEdit("1.0");
        mass2Edit = new QLineEdit("1.0");
        mass3Edit = new QLineEdit("1.0");

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
        triangleLayout->addWidget(massFrame);

        // ПАНЕЛЬ АНИМАЦИИ
        QFrame* animationFrame = new QFrame;
        animationFrame->setFrameStyle(QFrame::Box);
        animationFrame->setLineWidth(1);
        QVBoxLayout* animationFrameLayout = new QVBoxLayout(animationFrame);

        QLabel* animationTitle = new QLabel("Animation settings");
        animationTitle->setStyleSheet("QLabel { font-weight: bold; }");
        animationFrameLayout->addWidget(animationTitle);

        // Кнопки анимации
        QHBoxLayout* animationButtonLayout = new QHBoxLayout;
        animationModeButton = new QPushButton("Set Functions");
        animationModeButton->setFixedHeight(35);
        animationModeButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");
        animationModeButton->setEnabled(false);

        animationToggleButton = new QPushButton("Start");
        animationToggleButton->setFixedHeight(35);
        animationToggleButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");
        animationToggleButton->setEnabled(false);

        animationResetButton = new QPushButton("Reset");
        animationResetButton->setFixedHeight(35);
        animationResetButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");
        animationResetButton->setEnabled(false);

        QPushButton* stopDrawingButton = new QPushButton("Stop Drawing");
        stopDrawingButton->setFixedHeight(35);
        stopDrawingButton->setStyleSheet("QPushButton { padding: 8px; background-color: #e0e0e0; color: black; border: 1px solid #aaa; }");

        animationButtonLayout->addWidget(animationModeButton);
        animationButtonLayout->addWidget(animationToggleButton);
        animationButtonLayout->addWidget(animationResetButton);
        animationButtonLayout->addWidget(stopDrawingButton);
        animationButtonLayout->addStretch();

        animationFrameLayout->addLayout(animationButtonLayout);

        // Параметры анимации
        QHBoxLayout* animationParamsLayout = new QHBoxLayout;

        QLabel* maxTimeLabel = new QLabel("Max Time:");
        maxTimeEdit = new QLineEdit("20.0");
        QDoubleValidator* maxTimeValidator = new QDoubleValidator(1.0, 1000.0, 3, this);
        maxTimeValidator->setLocale(QLocale::C);
        maxTimeEdit->setValidator(maxTimeValidator);
        maxTimeEdit->setMaximumWidth(60);
        maxTimeEdit->setEnabled(true);

        QLabel* speedLabel = new QLabel("Speed:");
        speedEdit = new QLineEdit("1.0");
        QDoubleValidator* speedValidator = new QDoubleValidator(0.01, 100.0, 3, this);
        speedValidator->setLocale(QLocale::C);
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
        triangleLayout->addWidget(animationFrame);

        // Добавляем контейнер треугольника в левый сплиттер
        leftSplitter->addWidget(triangleContainer);

        // НИЖНЯЯ ЛЕВАЯ ЧАСТЬ - исходная комплексная плоскость
        QWidget* plane1Container = new QWidget;
        QVBoxLayout* plane1Layout = new QVBoxLayout(plane1Container);

        complexPlaneView1 = new ComplexPlaneView;
        complexPlaneView1->setMinimumSize(300, 250);
        plane1Layout->addWidget(complexPlaneView1);

        // Добавляем исходную плоскость в левый сплиттер
        leftSplitter->addWidget(plane1Container);

        // Устанавливаем пропорции левого сплиттера (60% треугольник, 40% плоскость)
        leftSplitter->setStretchFactor(0, 6);
        leftSplitter->setStretchFactor(1, 4);

        // Добавляем левый сплиттер в главный layout
        leftMainLayout->addWidget(leftSplitter);

        // ПРАВАЯ ЧАСТЬ - сфера и преобразованная плоскость
        QWidget* rightContainer = new QWidget;
        QVBoxLayout* rightMainLayout = new QVBoxLayout(rightContainer);

        // Создаем вертикальный сплиттер для правой части
        rightSplitter = new QSplitter(Qt::Vertical);

        // ВЕРХНЯЯ ПРАВАЯ ЧАСТЬ - сфера
        QWidget* sphereContainer = new QWidget;
        QVBoxLayout* sphereLayout = new QVBoxLayout(sphereContainer);

        sphereWidget = new SphereWidget;
        sphereWidget->setMinimumSize(400, 300);
        sphereLayout->addWidget(sphereWidget);

        // Добавляем сферу в правый сплиттер
        rightSplitter->addWidget(sphereContainer);

        // НИЖНЯЯ ПРАВАЯ ЧАСТЬ - преобразованная комплексная плоскость
        QWidget* plane2Container = new QWidget;
        QVBoxLayout* plane2Layout = new QVBoxLayout(plane2Container);

        complexPlaneView2 = new ComplexPlaneView2;
        complexPlaneView2->setMinimumSize(400, 250);
        plane2Layout->addWidget(complexPlaneView2);

        // Добавляем преобразованную плоскость в правый сплиттер
        rightSplitter->addWidget(plane2Container);

        // Устанавливаем пропорции правого сплиттера (50% сфера, 50% плоскость)
        rightSplitter->setStretchFactor(0, 5);
        rightSplitter->setStretchFactor(1, 5);

        // Добавляем правый сплиттер в главный layout
        rightMainLayout->addWidget(rightSplitter);

        // Добавляем левую и правую части в главный сплиттер
        mainSplitter->addWidget(leftContainer);
        mainSplitter->addWidget(rightContainer);

        // Устанавливаем пропорции главного сплиттера (50% левая часть, 50% правая часть)
        mainSplitter->setStretchFactor(0, 5);
        mainSplitter->setStretchFactor(1, 5);

        setCentralWidget(mainSplitter);
        setWindowTitle("Triangle-Sphere Projection System with 4-Branch Inverse Transform");
        resize(1400, 900);

        // СОЗДАЕМ ТАЙМЕР АНИМАЦИИ
        animationTimer = new QTimer(this);
        animationTimer->setInterval(50); // 20 FPS

        // ПОДКЛЮЧАЕМ СИГНАЛЫ И СЛОТЫ
        connect(scene, &TriangleScene::dragFinished, this, &MainWindow::onDragFinished);
        connect(scene, &TriangleScene::triangleUpdated, this, &MainWindow::updatePointCoordinates);
        connect(scene, &TriangleScene::pointPositionChanging, this, &MainWindow::updateSpherePoint);
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
        connect(autoScaleButton, &QPushButton::clicked, this, &MainWindow::autoScaleTriangleView);
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

        // Подключаем кнопку установки масс
        connect(setMassesButton, &QPushButton::clicked, this, &MainWindow::setMasses);
        connect(scene, &TriangleScene::massesChanged, sphereWidget, &SphereWidget::setMasses);

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
        connect(animationToggleButton, &QPushButton::clicked, this, &MainWindow::onAnimationToggle);
        connect(animationResetButton, &QPushButton::clicked, this, &MainWindow::onAnimationReset);
        connect(timeSlider, &QSlider::valueChanged, this, &MainWindow::onTimeSliderChanged);
        connect(animationTimer, &QTimer::timeout, this, &MainWindow::updateAnimation);

        // Подключаем чекбокс траектории к обоим виджетам
        connect(showTrajectoryCheckbox, &QCheckBox::toggled, sphereWidget, &SphereWidget::setShowTrajectory);
        connect(showTrajectoryCheckbox, &QCheckBox::toggled, complexPlaneView1, &ComplexPlaneView::setShowTrajectory);
        connect(showTrajectoryCheckbox, &QCheckBox::toggled, complexPlaneView2, &ComplexPlaneView2::setShowTrajectory);

        // Подключаем кнопку остановки рисования
        connect(stopDrawingButton, &QPushButton::clicked, this, [this, stopDrawingButton]() {
            bool drawingEnabled = sphereWidget->isDrawingEnabled();
            sphereWidget->setDrawingEnabled(!drawingEnabled);
            if (complexPlaneView1) complexPlaneView1->setDrawingEnabled(!drawingEnabled);
            if (complexPlaneView2) complexPlaneView2->setDrawingEnabled(!drawingEnabled);

            if (drawingEnabled) {
                stopDrawingButton->setText("Resume Drawing");
            } else {
                stopDrawingButton->setText("Stop Drawing");
            }
        });

        // Подключаем обработчики для полей ввода
        connect(maxTimeEdit, &QLineEdit::editingFinished, this, &MainWindow::fixMaxTimeInput);
        connect(speedEdit, &QLineEdit::editingFinished, this, &MainWindow::fixSpeedInput);

        // ПОДКЛЮЧАЕМ ЧЕКБОКС ANIMATION MODE
        connect(animationModeCheckbox, &QCheckBox::toggled, this, [this](bool checked) {
            isAnimationMode = checked;

            // Включаем/выключаем элементы управления анимацией
            animationModeButton->setEnabled(checked);
            animationToggleButton->setEnabled(checked);
            animationResetButton->setEnabled(checked);
            timeSlider->setEnabled(checked);

            // ВКЛЮЧАЕМ ПОЛЯ ВВОДА ДАЖЕ В РЕЖИМЕ АНИМАЦИИ
            maxTimeEdit->setEnabled(true);
            speedEdit->setEnabled(true);
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
                // СБРАСЫВАЕМ СОСТОЯНИЕ КНОПКИ
                isAnimationRunning = false;
                animationToggleButton->setText("Start");
            } else {
                // Выключаем режим анимации
                animationTimer->stop();
                isAnimationRunning = false;
                animationToggleButton->setText("Start");
            }
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

double MainWindow::calculateTriangleSize(const QList<QPointF>& points) {
    if (points.size() != 3) return 0.0;
    
    // Вычисляем длины сторон треугольника
    double side1 = QLineF(points[0], points[1]).length();
    double side2 = QLineF(points[1], points[2]).length();
    double side3 = QLineF(points[2], points[0]).length();
    
    // Возвращаем максимальную длину стороны как размер треугольника
    return qMax(side1, qMax(side2, side3));
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
            // Получаем сырые координаты (до нормализации)
            QVector3D rawCoords = CoordTransform::getRawSphereCoordinates(points, masses);
            QVector3D rawCoordsnorm = CoordTransform::transformToSphere(points, masses);
            if (!rawCoords.isNull()) {
                double radius = rawCoords.length();

                // Обновляем метку радиуса
                if (radiusLabel) {
                    QString radiusStr;
                    if (radius > 1000 || (radius < 0.001 && radius > 0)) {
                        radiusStr = QString::number(radius, 'e', 3);
                    } else {
                        radiusStr = QString::number(radius, 'f', 3);
                    }
                    radiusLabel->setText("Radius: " + radiusStr);
                }

                // Обновляем комплексную плоскость (ξ₂, ξ₃)
                if (complexPlaneView1) {
                    QPointF xiPoint(rawCoordsnorm.x(), rawCoordsnorm.y());
                    complexPlaneView1->setPoint(xiPoint);

                    if (showTrajectoryCheckbox->isChecked() && complexPlaneView1->isDrawingEnabled()) {
                        complexPlaneView1->addToTrajectory(xiPoint);
                    }
                }

                if (complexPlaneView2) {
                    QPointF xiPoint(rawCoordsnorm.x(), rawCoordsnorm.y());
                    QVector<ComplexSolution> solutions = CoordTransform::transformZetaToZ(xiPoint);
                    complexPlaneView2->setSolutions(solutions);

                    if (showTrajectoryCheckbox->isChecked() && complexPlaneView2->isDrawingEnabled()) {
                        complexPlaneView2->addToTrajectory(solutions);
                    }
                }

                // Обновляем метку комплексных координат
                if (complexCoordsLabel) {
                    complexCoordsLabel->setText(QString("Complex: (%1, %2)")
                                                    .arg(rawCoordsnorm.x(), 0, 'f', 3)
                                                    .arg(rawCoordsnorm.y(), 0, 'f', 3));
                }
            }

            // Получаем нормализованную точку для отображения на сфере
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

    // ПРИНУДИТЕЛЬНО ОБНОВЛЯЕМ SPHERE WIDGET
    sphereWidget->setMasses(masses);

    // Двойное обновление для надежности
    sphereWidget->update();
    QTimer::singleShot(50, sphereWidget, [this, masses]() {
        sphereWidget->setMasses(masses);
        sphereWidget->update();
    });

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

        animationToggleButton->setEnabled(true); // ИСПРАВЛЕНО: было animationStartButton
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

void MainWindow::onAnimationReset()
{
    if (animationTimer) {
        animationTimer->stop();
    }

    currentTime = 0.0;
    if (timeSlider) timeSlider->setValue(0);

    if (isAnimationMode) {
        evaluateFunctions(currentTime);
    }

    // СБРАСЫВАЕМ СОСТОЯНИЕ КНОПКИ
    isAnimationRunning = false;
    if (animationToggleButton) animationToggleButton->setText("Start");

    // Очищаем траекторию при сбросе
    if (sphereWidget) {
        sphereWidget->clearTrajectory();
    }
    if (complexPlaneView1) {
        complexPlaneView1->clearTrajectory();
    }
    if (complexPlaneView2) {
        complexPlaneView2->clearTrajectory();
    }
}

void MainWindow::onTimeSliderChanged(int value)
{
    if (!maxTimeEdit) return;

    bool ok;
    maxTime = maxTimeEdit->text().toDouble(&ok);
    if (!ok || maxTime <= 0) {
        maxTime = 20.0;
    }

    currentTime = (value / 100.0) * maxTime;
    evaluateFunctions(currentTime);
    updateTimeLabel();
}

void MainWindow::updateAnimation()
{
    if (!maxTimeEdit || !speedEdit) return;

    bool ok;
    maxTime = maxTimeEdit->text().toDouble(&ok);
    if (!ok || maxTime <= 0) {
        maxTime = 20.0;
    }

    double speedFactor = speedEdit->text().toDouble(&ok);
    if (!ok || speedFactor <= 0) {
        speedFactor = 1.0;
    }

    double previousTime = currentTime;
    currentTime += timeStep * speedFactor;

    // Если время перескочило с максимума на минимум, разрываем траектории
    if (previousTime > maxTime * 0.9 && currentTime < maxTime * 0.1) {
        if (sphereWidget && sphereWidget->isDrawingEnabled()) {
            sphereWidget->breakTrajectory();
        }
        if (complexPlaneView1 && complexPlaneView1->isDrawingEnabled()) {
            complexPlaneView1->breakTrajectory();
        }
        if (complexPlaneView2 && complexPlaneView2->isDrawingEnabled()) {
            complexPlaneView2->breakTrajectory();
        }
    }

    if (currentTime > maxTime) {
        currentTime = 0.0;
    }

    int sliderValue = static_cast<int>((currentTime / maxTime) * 100);
    if (timeSlider) {
        timeSlider->setValue(sliderValue);
    }

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
    if (!isAnimationMode || !scene) return;

    // ДОБАВЛЯЕМ ПРОВЕРКИ НА ПУСТЫЕ ФУНКЦИИ
    if (x1Func.isEmpty() || y1Func.isEmpty() || x2Func.isEmpty() ||
        y2Func.isEmpty() || x3Func.isEmpty() || y3Func.isEmpty()) {
        return;
    }

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

    // Проверяем валидность точек
    for (const QPointF& point : points) {
        if (std::isnan(point.x()) || std::isinf(point.x()) ||
            std::isnan(point.y()) || std::isinf(point.y())) {
            qWarning() << "Invalid point in animation:" << point;
            return;
        }
    }

    // ПРОВЕРЯЕМ РАЗМЕР ТРЕУГОЛЬНИКА И МАСШТАБИРУЕМ ТОЛЬКО МАЛЕНЬКИЕ
    double triangleSize = calculateTriangleSize(points);
    const double smallTriangleThreshold = 50.0;

    if (triangleSize < smallTriangleThreshold) {
        const double scale = 50.0;
        const double center = 100.0;

        for (int i = 0; i < points.size(); ++i) {
            points[i] = QPointF(points[i].x() * scale + center,
                                points[i].y() * scale + center);
        }
    }

    blockSceneUpdates = true;
    scene->setPoints(points);
    blockSceneUpdates = false;

    // Автоматически масштабируем вид
    autoScaleTriangleView();

    // ОБНОВЛЯЕМ РАДИУС И КОМПЛЕКСНЫЕ ПЛОСКОСТИ
    auto masses = scene->getMasses();
    QVector3D rawCoords = CoordTransform::getRawSphereCoordinates(points, masses);
    QVector3D rawCoordsnorm = CoordTransform::transformToSphere(points, masses);

    if (!rawCoords.isNull()) {
        double radius = rawCoords.length();

        if (radiusLabel) {
            QString radiusStr;
            if (radius > 1000 || (radius < 0.001 && radius > 0)) {
                radiusStr = QString::number(radius, 'e', 3);
            } else {
                radiusStr = QString::number(radius, 'f', 3);
            }
            radiusLabel->setText("Radius: " + radiusStr);
        }

        // Обновляем исходную комплексную плоскость
        if (complexPlaneView1) {
            complexPlaneView1->setPoint(QPointF(rawCoordsnorm.x(), rawCoordsnorm.y()));

            if (showTrajectoryCheckbox && showTrajectoryCheckbox->isChecked() && complexPlaneView1->isDrawingEnabled()) {
                complexPlaneView1->addToTrajectory(QPointF(rawCoordsnorm.x(), rawCoordsnorm.y()));
            }
        }

        // Обновляем преобразованную плоскость
        if (complexPlaneView2) {
            QPointF xiPoint(rawCoordsnorm.x(), rawCoordsnorm.y());
            QVector<ComplexSolution> solutions = CoordTransform::transformZetaToZ(xiPoint);
            complexPlaneView2->setSolutions(solutions);

            if (showTrajectoryCheckbox && showTrajectoryCheckbox->isChecked() && complexPlaneView2->isDrawingEnabled()) {
                complexPlaneView2->addToTrajectory(solutions);
            }
        }

        if (complexCoordsLabel) {
            complexCoordsLabel->setText(QString("Complex: (%1, %2)")
                                            .arg(rawCoords.x(), 0, 'f', 3)
                                            .arg(rawCoords.y(), 0, 'f', 3));
        }
    }

    // ВЫЧИСЛЯЕМ ТОЧКУ НА СФЕРЕ
    QVector3D spherePoint = CoordTransform::transformToSphere(points, masses);

    if (!spherePoint.isNull() && sphereWidget) {
        // Обновляем точку на сфере
        sphereWidget->setPoint(spherePoint);

        // Добавляем в траекторию если включено отображение
        if (showTrajectoryCheckbox && showTrajectoryCheckbox->isChecked()) {
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

void MainWindow::autoScaleTriangleView() {
    if (!view || !scene) return;

    // Получаем bounding rect всех элементов сцены (включая подписи)
    QRectF rect = scene->itemsBoundingRect();
    if (rect.isEmpty()) return;

    // ДОБАВЛЯЕМ ПОЛЯ ДЛЯ ПОДПИСЕЙ
    // Учитываем, что подписи могут быть справа и сверху от точек
    const double labelMargin = 60.0; // Отступ для подписей (эмпирически подобран)
    
    // Расширяем rect чтобы вместить подписи
    rect.adjust(-20, -labelMargin, labelMargin, 20);

    // Дополнительно проверяем размер треугольника
    auto points = scene->getPoints();
    double triangleSize = calculateTriangleSize(points);
    const double smallSizeThreshold = 50.0;

    if (triangleSize < smallSizeThreshold) {
        // Для маленьких треугольников - дополнительные margins
        double margin = 1.5; // 150% margins для маленьких треугольников
        double marginX = rect.width() * margin;
        double marginY = rect.height() * margin;
        rect.adjust(-marginX, -marginY, marginX, marginY);
    } else {
        // Для нормальных треугольников - стандартные margins
        double margin = 0.4; // 40% margins
        double marginX = rect.width() * margin;
        double marginY = rect.height() * margin;
        rect.adjust(-marginX, -marginY, marginX, marginY);
    }

    // ГАРАНТИРУЕМ МИНИМАЛЬНЫЙ РАЗМЕР С УЧЕТОМ ПОДПИСЕЙ
    const double minViewWidth = 180.0;
    const double minViewHeight = 180.0;
    
    if (rect.width() < minViewWidth) {
        double expand = (minViewWidth - rect.width()) / 2.0;
        rect.adjust(-expand, 0, expand, 0);
    }
    if (rect.height() < minViewHeight) {
        double expand = (minViewHeight - rect.height()) / 2.0;
        rect.adjust(0, -expand, 0, expand);
    }

    // Сбрасываем трансформацию и масштабируем
    view->resetTransform();
    view->fitInView(rect, Qt::KeepAspectRatio);
    
    // Центрируем
    view->centerOn(rect.center());
}

void MainWindow::onAnimationToggle()
{
    if (!animationTimer) {
        qWarning() << "Animation timer is null";
        return;
    }

    if (!isAnimationRunning) {
        // Запускаем анимацию
        animationTimer->start();
        isAnimationRunning = true;
        if (animationToggleButton) animationToggleButton->setText("Pause");
    } else {
        // Останавливаем анимацию
        animationTimer->stop();
        isAnimationRunning = false;
        if (animationToggleButton) animationToggleButton->setText("Start");
    }
}
