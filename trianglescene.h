#ifndef TRIANGLESCENE_H
#define TRIANGLESCENE_H

#include <QGraphicsScene>
#include <QList>
#include <QPointF>
#include "dragpoint.h"

class TriangleScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit TriangleScene(QObject* parent = nullptr);
    ~TriangleScene();

    void updateTriangle();
    QList<QPointF> getPoints() const;
    QList<double> getMasses() const;
    QString getPointCoordinates() const;
    void setPoints(const QList<QPointF>& points);

signals:
    void triangleUpdated();
    void coordinatesUpdated(const QString& coords);
    void dragFinished(); // Добавляем сигнал окончания перетаскивания

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    QList<DragPoint*> points;
    QList<QGraphicsLineItem*> lines;
    QList<QGraphicsSimpleTextItem*> labels;
};

#endif // TRIANGLESCENE_H
