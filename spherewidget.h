#ifndef SPHEREWIDGET_H
#define SPHEREWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QMouseEvent>
#include <QWheelEvent>
#include "dragpoint.h"

class SphereWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    void setMasses(const QList<double>& masses);
    explicit SphereWidget(QWidget* parent = nullptr);
    ~SphereWidget();

    void setPoint(const QVector3D& point);
    QVector3D getPoint() const;
    bool isValid() const { return context() && context()->isValid(); }

    // Новый метод для автоматического вращения к точке
    void rotateToPoint(const QVector3D& point);

    // Новый метод для установки вращения
    void setRotation(const QQuaternion& newRotation) {
        rotation = newRotation;
        update();
    }

    void setRotationMode(bool mode) {
        rotationMode = mode;
        update();
    }
    bool getRotationMode() const { return rotationMode; }

signals:
    void spherePointClicked(const QVector3D& point);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QVector3D spherePoint;
    QMatrix4x4 projection;
    QMatrix4x4 modelView;
    QQuaternion rotation;
    QPoint lastMousePos;
    QList<double> m_masses;
    float distance;
    bool isDraggingPoint;
    bool isRotatingSphere;
    bool rotationMode = true; // true - вращение сферы, false - перемещение точки

    void setupLighting();
    void drawSphere();
    void drawCoordinateSystem();
    void drawPoint();
    void drawSpecialLines();
    void drawCollisionPoints();
    void drawPoles();
    void drawEquilateralPoints();
    QVector3D getSpherePointFromMouse(const QPoint& mousePos) const;
    QVector3D projectToScreen(const QVector3D& point) const;
};

#endif // SPHEREWIDGET_H
