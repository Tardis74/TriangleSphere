#ifndef COMPLEXPLANEVIEW2_H
#define COMPLEXPLANEVIEW2_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointF>
#include <QVector>
#include <QColor>
#include "coordtransform.h" // Включаем полное определение ComplexSolution

class ComplexPlaneView2 : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ComplexPlaneView2(QWidget *parent = nullptr);

    void setSolutions(const QVector<ComplexSolution>& solutions);
    void addToTrajectory(const QVector<ComplexSolution>& solutions);
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
    void updatePoints();
    void updateTrajectory();
    QPointF sceneToComplex(const QPointF& scenePoint) const;
    QPointF complexToScene(const QPointF& complexPoint) const;

    QGraphicsScene* m_scene;
    QVector<ComplexSolution> m_currentSolutions;
    bool m_showTrajectory;

    // Траектории для каждой ветви отдельно
    QVector<QVector<QPointF>> m_trajectoryBranches[4]; // 4 ветви
    QVector<QColor> m_branchColors;

    bool m_drawingEnabled;

    // Графические элементы - теперь массив точек
    QVector<QGraphicsEllipseItem*> m_pointItems;
    QVector<QGraphicsPathItem*> m_trajectoryItems;

    // Область отображения
    const double m_minValue = -3.0;
    const double m_maxValue = 3.0;
};

#endif // COMPLEXPLANEVIEW2_H
