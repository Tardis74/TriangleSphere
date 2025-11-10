#ifndef COMPLEXPLANEVIEW_H
#define COMPLEXPLANEVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointF>
#include <QVector>

class ComplexPlaneView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ComplexPlaneView(QWidget *parent = nullptr);

    void setPoint(const QPointF& point);
    void addToTrajectory(const QPointF& point);
    void clearTrajectory();
    void setShowTrajectory(bool show);
    void breakTrajectory();
    void setDrawingEnabled(bool enabled) { m_drawingEnabled = enabled; }
    bool isDrawingEnabled() const { return m_drawingEnabled; }

protected:
    void drawForeground(QPainter* painter, const QRectF& rect) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void createCoordinateSystem();
    void updatePoint();
    void updateTrajectory();
    QPointF sceneToComplex(const QPointF& scenePoint) const;
    QPointF complexToScene(const QPointF& complexPoint) const;

    QGraphicsScene* m_scene;
    QPointF m_currentPoint;
    bool m_showTrajectory;
    QVector<QPointF> m_trajectoryPoints;
    QVector<QVector<QPointF>> m_trajectorySegments;
    bool m_drawingEnabled;

    // Графические элементы
    QGraphicsEllipseItem* m_pointItem;
    QGraphicsPathItem* m_trajectoryItem;

    // Область отображения в комплексных координатах
    const double m_minValue = -1.0;
    const double m_maxValue = 1.0;
};

#endif // COMPLEXPLANEVIEW_H
