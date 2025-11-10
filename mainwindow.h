#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QLineEdit>
#include <QFrame>
#include <QTimer>
#include <QSlider>
#include <QGroupBox>
#include <atomic>
#include <mutex>
#include "trianglescene.h"
#include "spherewidget.h"
#include "complexplaneview.h"
#include "complexplaneview2.h"
#include <QCheckBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private slots:
    void fixMaxTimeInput();
    void fixSpeedInput();
    void autoScaleView();
    void updateSpherePoint();
    void handleSpherePointClicked(const QVector3D& point);
    void onDragFinished();
    void zoomIn();
    void zoomOut();
    void setMasses();
    void updatePointCoordinates();

    // Animation slots
    void onAnimationModeClicked();
    void onAnimationReset();
    void onTimeSliderChanged(int value);
    void updateAnimation();
    void updateTimeLabel();

private:
    double calculateTriangleSize(const QList<QPointF>& points);

    // Инициализируем все указатели nullptr
    QLabel* equilateralPointsLabel = nullptr;
    QCheckBox* showTrajectoryCheckbox = nullptr;
    QLineEdit* maxTimeEdit = nullptr;
    QLineEdit* speedEdit = nullptr;

    QSplitter* mainSplitter = nullptr;
    QSplitter* rightSplitter = nullptr;
    QSplitter* leftSplitter = nullptr;
    SphereWidget* sphereWidget = nullptr;
    ComplexPlaneView* complexPlaneView1 = nullptr;
    ComplexPlaneView2* complexPlaneView2 = nullptr;
    QGraphicsView* view = nullptr;
    TriangleScene* scene = nullptr;
    QLabel* sphereCoordsLabel = nullptr;
    QLabel* coordsLabel = nullptr;
    QLabel* timeLabel = nullptr;
    QLabel* complexCoordsLabel = nullptr;
    QLabel* radiusLabel = nullptr;
    std::atomic<bool> blockSceneUpdates;
    QVector3D lastSpherePoint;
    const double updateThreshold = 0.001;

    std::mutex updateMutex;
    bool updatingFromTriangle = false;
    bool updatingFromSphere = false;

    // Элементы для ввода масс
    QLineEdit* mass1Edit = nullptr;
    QLineEdit* mass2Edit = nullptr;
    QLineEdit* mass3Edit = nullptr;
    QPushButton* setMassesButton = nullptr;

    // Кнопки управления
    QPushButton* zoomInButton = nullptr;
    QPushButton* zoomOutButton = nullptr;
    QPushButton* resetButton = nullptr;
    QPushButton* toggleModeButton = nullptr;

    // Анимация
    QPushButton* animationModeButton = nullptr;
    QPushButton* animationToggleButton = nullptr;
    QPushButton* animationResetButton = nullptr;
    QSlider* timeSlider = nullptr;
    QTimer* animationTimer = nullptr;

    // Переменные анимации
    bool isAnimationMode = false;
    bool isAnimationRunning = false;
    double currentTime = 0.0;
    double timeStep = 0.1;
    double maxTime = 10.0;

    QString x1Func, y1Func, x2Func, y2Func, x3Func, y3Func;

    void evaluateFunctions(double t);
    double evaluateExpression(const QString& expression, double t);
    void autoScaleTriangleView();
    void onAnimationToggle(); // Переносим объявление сюда
};

#endif // MAINWINDOW_H
