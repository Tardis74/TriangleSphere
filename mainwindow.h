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
    void onAnimationStart();
    void onAnimationPause();
    void onAnimationReset();
    void onTimeSliderChanged(int value);
    void updateAnimation();
    void updateTimeLabel();

private:
    QLabel* equilateralPointsLabel = nullptr;
    QCheckBox* showTrajectoryCheckbox = nullptr;
    QLineEdit* maxTimeEdit = nullptr;
    QLineEdit* speedEdit = nullptr;

    QSplitter* mainSplitter = nullptr;
    SphereWidget* sphereWidget = nullptr;
    QGraphicsView* view = nullptr;
    TriangleScene* scene = nullptr;
    QLabel* sphereCoordsLabel = nullptr;
    QLabel* coordsLabel = nullptr;
    QLabel* timeLabel = nullptr;
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
    QPushButton* animationStartButton = nullptr;
    QPushButton* animationPauseButton = nullptr;
    QPushButton* animationResetButton = nullptr;
    QSlider* timeSlider = nullptr;
    QTimer* animationTimer = nullptr;

    bool isAnimationMode = false;
    double currentTime = 0.0;
    double timeStep = 0.1;
    double maxTime = 10.0;

    QString x1Func, y1Func, x2Func, y2Func, x3Func, y3Func;

    void evaluateFunctions(double t);
    double evaluateExpression(const QString& expression, double t);
};

#endif // MAINWINDOW_H
