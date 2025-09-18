#include "dragpoint.h"
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

DragPoint::DragPoint(qreal x, qreal y, qreal size, double mass)
    : QGraphicsEllipseItem(-size/2, -size/2, size, size), m_mass(mass) {
    setPos(x, y);
    setBrush(QBrush(Qt::red));
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
}

QVariant DragPoint::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionHasChanged) {
        QPointF newPos = value.toPointF();

        // Ограничиваем максимальные координаты
        const qreal maxCoord = 10000.0;
        if (std::abs(newPos.x()) > maxCoord || std::abs(newPos.y()) > maxCoord) {
            // Игнорируем изменение, если координаты слишком большие
            return QGraphicsEllipseItem::itemChange(change, pos());
        }

        emit positionChanged();
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}

void DragPoint::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    emit dragFinished();
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}

double DragPoint::mass() const {
    return m_mass;
}

void DragPoint::setMass(double mass) {
    if (qFuzzyCompare(m_mass, mass))
        return;

    // Check for valid mass
    if (std::isnan(mass) || std::isinf(mass)) {
        qWarning() << "Attempt to set invalid mass:" << mass;
        return;
    }

    // Check for non-positive mass
    if (mass <= 0) {
        qWarning() << "Attempt to set non-positive mass:" << mass;
        return;
    }

    m_mass = mass;
    emit massChanged(mass);
}
