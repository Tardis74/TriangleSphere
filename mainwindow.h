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
#include <atomic>
#include <mutex>
#include "trianglescene.h"
#include "spherewidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private slots:
    void autoScaleView();
    void updateSpherePoint();
    void handleSpherePointClicked(const QVector3D& point);
    void onDragFinished();
    void zoomIn();
    void zoomOut();
    void setMasses();
    void updatePointCoordinates();

private:
    QSplitter* mainSplitter = nullptr;
    SphereWidget* sphereWidget = nullptr;
    QGraphicsView* view = nullptr;
    TriangleScene* scene = nullptr;
    QLabel* sphereCoordsLabel = nullptr;
    QLabel* coordsLabel = nullptr; // Метка для координат точек
    std::atomic<bool> blockSceneUpdates;
    QVector3D lastSpherePoint;
    const double updateThreshold = 0.01;

    std::mutex updateMutex;
    bool updatingFromTriangle = false;
    bool updatingFromSphere = false;

    // Элементы для ввода масс
    QLineEdit* mass1Edit = nullptr;
    QLineEdit* mass2Edit = nullptr;
    QLineEdit* mass3Edit = nullptr;
    QPushButton* setMassesButton = nullptr;

    QPushButton* zoomInButton = nullptr;
    QPushButton* zoomOutButton = nullptr;
    QPushButton* resetButton = nullptr;
    QPushButton* toggleModeButton = nullptr; // Добавляем эту строку
};

#endif // MAINWINDOW_H
