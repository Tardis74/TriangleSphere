#ifndef COMPLEXPLANESCENE_H
#define COMPLEXPLANESCENE_H

#include <QGraphicsScene>
#include <QPointF>
#include <QVector>

class ComplexPlaneScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit ComplexPlaneScene(QObject *parent = nullptr);
    ~ComplexPlaneScene();

    void setPoint(const QPointF& point);
    void addToTrajectory(const QPointF& point);
    void clearTrajectory();
    void setShowTrajectory(bool show);
    void breakTrajectory();
    void setDrawingEnabled(bool enabled) { m_drawingEnabled = enabled; }
    bool isDrawingEnabled() const { return m_drawingEnabled; }

    // Новый метод для обновления системы координат
    void updateCoordinateSystem(const QRectF& viewportRect);

private:
    void updatePoint();
    void updateTrajectory();

    QPointF m_currentPoint;
    bool m_showTrajectory;
    QVector<QPointF> m_trajectoryPoints;
    QVector<QVector<QPointF>> m_trajectorySegments;
    bool m_drawingEnabled;

    // Графические элементы
    QGraphicsEllipseItem* m_pointItem;
    QGraphicsPathItem* m_trajectoryItem;

    // Элементы системы координат (динамически обновляются)
    QVector<QGraphicsItem*> m_axisItems;
};

#endif // COMPLEXPLANESCENE_H
