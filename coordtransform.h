#ifndef COORDTRANSFORM_H
#define COORDTRANSFORM_H

#include <QVector3D>
#include <QPointF>
#include <QList>
#include <QVector>

class CoordTransform {
public:
    static QVector3D transformToSphere(const QList<QPointF>& points, const QList<double>& masses);
    static QVector<QPointF> transformFromSphere(const QVector3D& spherePoint, const QList<double>& masses, double scale = 300.0);
};

#endif // COORDTRANSFORM_H
