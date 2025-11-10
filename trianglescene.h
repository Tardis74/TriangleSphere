#ifndef TRIANGLESCENE_H
#define TRIANGLESCENE_H

#include <QGraphicsScene>
#include <QList>
#include <QPointF>
#include "dragpoint.h"

class TriangleScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit TriangleScene(QObject* parent = nullptr);
    ~TriangleScene();

    void updateTriangle();
    QList<QPointF> getPoints() const;
    QList<double> getMasses() const;
    void setPoints(const QList<QPointF>& points);
    void setMasses(const QList<double>& masses);
    QRectF itemsBoundingRect() const;

signals:
    void triangleUpdated();
    void dragFinished();
    void sceneDragStarted();
    void sceneDragFinished();
    void pointPositionChanging();
    void massesChanged(const QList<double>& masses);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    QList<DragPoint*> points;
    QList<QGraphicsLineItem*> lines;
    QList<QGraphicsSimpleTextItem*> labels;

    bool isDraggingScene = false;
    QPointF lastDragPos;
};

#endif // TRIANGLESCENE_H
