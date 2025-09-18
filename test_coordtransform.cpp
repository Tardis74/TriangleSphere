#include <QtTest>
#include "coordtransform.h"

class TestCoordTransform : public QObject
{
    Q_OBJECT

private slots:
    void testTransformToSphere();
    void testTransformFromSphere();
    void testRoundTrip();
    void testInvalidInputs();
};

void TestCoordTransform::testTransformToSphere()
{
    QList<QPointF> points = {QPointF(0, 0), QPointF(1, 0), QPointF(0, 1)};
    QList<double> masses = {1.0, 1.0, 1.0};

    QVector3D result = CoordTransform::transformToSphere(points, masses);
    
    // Проверяем, что результат не нулевой и нормализованный
    QVERIFY(!result.isNull());
    QVERIFY(qFuzzyCompare(result.length(), 1.0f));
    
    qDebug() << "testTransformToSphere passed. Result:" << result;
}

void TestCoordTransform::testTransformFromSphere()
{
    QVector3D spherePoint(1, 0, 0);
    QList<double> masses = {1.0, 1.0, 1.0};

    auto result = CoordTransform::transformFromSphere(spherePoint, masses);
    
    QCOMPARE(result.size(), 3);
    // Проверяем, что точки не содержат NaN или Inf
    for (const auto& point : result) {
        QVERIFY(!std::isnan(point.x()) && !std::isinf(point.x()));
        QVERIFY(!std::isnan(point.y()) && !std::isinf(point.y()));
    }
    
    qDebug() << "testTransformFromSphere passed. Result:" << result;
}

void TestCoordTransform::testRoundTrip()
{
    // Тестируем преобразование туда и обратно
    QList<QPointF> originalPoints = {QPointF(100, 100), QPointF(200, 100), QPointF(150, 200)};
    QList<double> masses = {1.0, 1.0, 1.0};

    QVector3D spherePoint = CoordTransform::transformToSphere(originalPoints, masses);
    QVERIFY(!spherePoint.isNull());

    auto newPoints = CoordTransform::transformFromSphere(spherePoint, masses);
    QCOMPARE(newPoints.size(), 3);

    // Проверяем, что преобразование обратно дает близкие к исходным точки
    for (int i = 0; i < 3; ++i) {
        double dist = QLineF(originalPoints[i], newPoints[i]).length();
        QVERIFY(dist < 100); // Эмпирически подобранное значение
    }
    
    qDebug() << "testRoundTrip passed. Original points:" << originalPoints;
    qDebug() << "Transformed points:" << newPoints;
}

void TestCoordTransform::testInvalidInputs()
{
    // Тестируем обработку неверных входных данных
    QList<QPointF> emptyPoints;
    QList<double> emptyMasses;
    
    QVector3D result1 = CoordTransform::transformToSphere(emptyPoints, emptyMasses);
    QVERIFY(result1.isNull());
    
    QList<QPointF> points = {QPointF(0, 0), QPointF(1, 0)}; // Недостаточно точек
    QList<double> masses = {1.0, 1.0};
    
    QVector3D result2 = CoordTransform::transformToSphere(points, masses);
    QVERIFY(result2.isNull());
    
    qDebug() << "testInvalidInputs passed";
}

QTEST_APPLESS_MAIN(TestCoordTransform)
#include "test_coordtransform.moc"