#ifndef COORDTRANSFORM_H
#define COORDTRANSFORM_H

#include <QVector3D>
#include <QPointF>
#include <QList>
#include <QVector>
#include <QColor>

// Полное определение структуры для хранения решения с цветом
struct ComplexSolution {
    QPointF point;
    QColor color;
    int branch; // номер ветви (0-3)

    ComplexSolution() : point(0, 0), color(Qt::black), branch(0) {}
    ComplexSolution(const QPointF& p, const QColor& c, int b) : point(p), color(c), branch(b) {}
};

class CoordTransform {
public:
    static QVector3D transformToSphere(const QList<QPointF>& points, const QList<double>& masses);
    static QVector<QPointF> transformFromSphere(const QVector3D& spherePoint, const QList<double>& masses, double scale = 300.0);
    static QVector3D getRawSphereCoordinates(const QList<QPointF>& points, const QList<double>& masses);

    // Новые методы для преобразования ζ -> z с несколькими решениями
    static QVector<ComplexSolution> transformZetaToZ(const QPointF& zetaPoint);
    static QPointF transformZToZeta(const QPointF& zPoint);
};

#endif // COORDTRANSFORM_H
