#ifndef DRAGPOINT_H
#define DRAGPOINT_H

#include <QGraphicsEllipseItem>
#include <QObject>

class DragPoint : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT
    Q_PROPERTY(double mass READ mass WRITE setMass NOTIFY massChanged)
public:
    DragPoint(qreal x, qreal y, qreal size = 10, double mass = 1.0);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    double mass() const;
    void setMass(double mass);

    void setPosSilent(const QPointF& pos) {
        setFlag(ItemSendsGeometryChanges, false);
        setPos(pos);
        setFlag(ItemSendsGeometryChanges, true);
    }

signals:
    void positionChanged();
    void dragFinished();
    void massChanged(double mass);
    void positionChanging();

private:
    double m_mass;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif // DRAGPOINT_H
