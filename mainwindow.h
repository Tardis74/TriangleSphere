#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QPlainTextEdit>
#include <atomic>
#include <mutex>
#include "trianglescene.h"
#include "spherewidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

private slots:
    void updateCoordinatesText(const QString& coords);
    void autoScaleView();
    void updateSpherePoint();
    void handleSpherePointClicked(const QVector3D& point);
    void onDragFinished(); // Новый слот для обработки окончания перетаскивания

private:
    SphereWidget* sphereWidget;
    QGraphicsView* view;
    TriangleScene* scene;
    QPlainTextEdit* coordinatesTextEdit;
    bool updatingTextFromScene;
    std::atomic<bool> blockSceneUpdates;
    QVector3D lastSpherePoint;
    const double updateThreshold = 0.01;

    // Добавляем мьютекс для синхронизации
    std::mutex updateMutex;

    // Добавляем флаги для предотвращения цикличных обновлений
    bool updatingFromTriangle;
    bool updatingFromSphere;

    void wheelEvent(QWheelEvent* event) override;
};

#endif // MAINWINDOW_H
