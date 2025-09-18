#ifndef SPHEREWIDGET_H
#define SPHEREWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QMouseEvent>
#include <QWheelEvent>

class SphereWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit SphereWidget(QWidget* parent = nullptr);
    ~SphereWidget();

    void setPoint(const QVector3D& point);
    QVector3D getPoint() const; // Объявляем без inline реализации
    bool isValid() const { return context() && context()->isValid(); }

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
    float distance;
    bool isDraggingPoint;

    void drawSphere();
    void drawCoordinateSystem();
    void drawPoint();
    QVector3D getSpherePointFromMouse(const QPoint& mousePos) const;
};

#endif // SPHEREWIDGET_H
