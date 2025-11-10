#include "complexplanescene.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QPainterPath>
#include <QDebug>

ComplexPlaneScene::ComplexPlaneScene(QObject *parent)
    : QGraphicsScene(parent), m_showTrajectory(false), m_drawingEnabled(true),
    m_pointItem(nullptr), m_trajectoryItem(nullptr)
{
    // Устанавливаем очень большой размер сцены
    setSceneRect(-10000, -10000, 20000, 20000);

    // Создаем элемент для точки
    m_pointItem = new QGraphicsEllipseItem(-4, -4, 8, 8);
    m_pointItem->setBrush(QBrush(Qt::red));
    m_pointItem->setPen(QPen(Qt::black));
    m_pointItem->setZValue(10);
    addItem(m_pointItem);
    m_pointItem->setVisible(false);

    // Создаем элемент для траектории
    m_trajectoryItem = new QGraphicsPathItem;
    m_trajectoryItem->setPen(QPen(Qt::blue, 2));
    m_trajectoryItem->setZValue(5);
    addItem(m_trajectoryItem);
    m_trajectoryItem->setVisible(false);

    m_trajectorySegments.append(QVector<QPointF>());
}

ComplexPlaneScene::~ComplexPlaneScene()
{
}

// Новый метод для обновления координатных осей на основе видимой области
void ComplexPlaneScene::updateCoordinateSystem(const QRectF& viewportRect)
{
    // Очищаем старые элементы осей
    for (auto item : m_axisItems) {
        removeItem(item);
        delete item;
    }
    m_axisItems.clear();

    // Получаем границы видимой области в координатах сцены
    double left = viewportRect.left();
    double right = viewportRect.right();
    double top = viewportRect.top();
    double bottom = viewportRect.bottom();
    double width = viewportRect.width();
    double height = viewportRect.height();

    // Создаем рамку вокруг видимой области
    QGraphicsRectItem* border = new QGraphicsRectItem(viewportRect);
    border->setPen(QPen(Qt::black, 2));
    border->setZValue(1);
    addItem(border);
    m_axisItems.append(border);

    // Ось X (нижняя граница) - ξ₂
    QGraphicsLineItem* xAxis = new QGraphicsLineItem(left, bottom, right, bottom);
    xAxis->setPen(QPen(Qt::black, 2));
    xAxis->setZValue(2);
    addItem(xAxis);
    m_axisItems.append(xAxis);

    // Ось Y (левая граница) - ξ₃
    QGraphicsLineItem* yAxis = new QGraphicsLineItem(left, top, left, bottom);
    yAxis->setPen(QPen(Qt::black, 2));
    yAxis->setZValue(2);
    addItem(yAxis);
    m_axisItems.append(yAxis);

    // Подписи осей
    QGraphicsSimpleTextItem* xLabel = new QGraphicsSimpleTextItem("ξ₂");
    xLabel->setPos(right - 20, bottom - 25);
    xLabel->setZValue(3);
    addItem(xLabel);
    m_axisItems.append(xLabel);

    QGraphicsSimpleTextItem* yLabel = new QGraphicsSimpleTextItem("ξ₃");
    yLabel->setPos(left + 5, top + 5);
    yLabel->setZValue(3);
    addItem(yLabel);
    m_axisItems.append(yLabel);

    // Вычисляем шаг для делений (примерно каждые 50 пикселей)
    double xStep = qMax(50.0, width / 10.0);
    double yStep = qMax(50.0, height / 10.0);

    // Деления и подписи на оси X
    for (double x = left + xStep; x < right; x += xStep) {
        // Засечка
        QGraphicsLineItem* tick = new QGraphicsLineItem(x, bottom - 5, x, bottom + 5);
        tick->setPen(QPen(Qt::black, 1));
        tick->setZValue(2);
        addItem(tick);
        m_axisItems.append(tick);

        // Подпись значения (переводим координаты сцены в значения ξ₂)
        double xi2 = x / 100.0; // Масштабируем для разумных значений
        QGraphicsSimpleTextItem* label = new QGraphicsSimpleTextItem(QString::number(xi2, 'f', 1));
        label->setPos(x - 10, bottom + 8);
        label->setZValue(3);
        QFont font = label->font();
        font.setPointSize(8);
        label->setFont(font);
        addItem(label);
        m_axisItems.append(label);
    }

    // Деления и подписи на оси Y
    for (double y = top + yStep; y < bottom; y += yStep) {
        // Засечка
        QGraphicsLineItem* tick = new QGraphicsLineItem(left - 5, y, left + 5, y);
        tick->setPen(QPen(Qt::black, 1));
        tick->setZValue(2);
        addItem(tick);
        m_axisItems.append(tick);

        // Подпись значения (переводим координаты сцены в значения ξ₃)
        double xi3 = -y / 100.0; // Инвертируем и масштабируем
        QGraphicsSimpleTextItem* label = new QGraphicsSimpleTextItem(QString::number(xi3, 'f', 1));
        label->setPos(left - 35, y - 8);
        label->setZValue(3);
        QFont font = label->font();
        font.setPointSize(8);
        label->setFont(font);
        addItem(label);
        m_axisItems.append(label);
    }

    // Подпись начала координат если оно в видимой области
    if (viewportRect.contains(0, 0)) {
        QGraphicsSimpleTextItem* origin = new QGraphicsSimpleTextItem("0");
        origin->setPos(left + 5, bottom - 20);
        origin->setZValue(3);
        QFont font = origin->font();
        font.setPointSize(8);
        origin->setFont(font);
        addItem(origin);
        m_axisItems.append(origin);
    }
}

void ComplexPlaneScene::setPoint(const QPointF& point)
{
    m_currentPoint = point;
    updatePoint();
}

void ComplexPlaneScene::updatePoint()
{
    if (!m_pointItem) return;

    if (m_currentPoint.isNull()) {
        m_pointItem->setVisible(false);
        return;
    }

    // Переводим значения ξ₂, ξ₃ в координаты сцены
    double x = m_currentPoint.x() * 100.0;  // ξ₂
    double y = -m_currentPoint.y() * 100.0; // ξ₃ (инвертируем)

    m_pointItem->setRect(x - 4, y - 4, 8, 8);
    m_pointItem->setVisible(true);
}

void ComplexPlaneScene::addToTrajectory(const QPointF& point)
{
    if (!m_drawingEnabled) return;

    if (m_trajectorySegments.isEmpty()) {
        m_trajectorySegments.append(QVector<QPointF>());
    }

    m_trajectorySegments.last().append(point);

    // Ограничиваем длину для производительности
    if (m_trajectorySegments.last().size() > 1000) {
        m_trajectorySegments.last().removeFirst();
    }

    updateTrajectory();
}

void ComplexPlaneScene::updateTrajectory()
{
    if (!m_trajectoryItem) return;

    QPainterPath path;

    for (const auto& segment : m_trajectorySegments) {
        if (segment.size() < 2) continue;

        // Начинаем путь с первой точки
        QPointF firstPoint = segment.first();
        path.moveTo(firstPoint.x() * 100.0, -firstPoint.y() * 100.0);

        // Добавляем линии к остальным точкам
        for (int i = 1; i < segment.size(); ++i) {
            const QPointF& point = segment[i];
            path.lineTo(point.x() * 100.0, -point.y() * 100.0);
        }
    }

    m_trajectoryItem->setPath(path);
    m_trajectoryItem->setVisible(m_showTrajectory && !path.isEmpty());
}

void ComplexPlaneScene::clearTrajectory()
{
    m_trajectorySegments.clear();
    m_trajectorySegments.append(QVector<QPointF>());
    updateTrajectory();
}

void ComplexPlaneScene::setShowTrajectory(bool show)
{
    m_showTrajectory = show;
    updateTrajectory();
}

void ComplexPlaneScene::breakTrajectory()
{
    if (!m_trajectorySegments.isEmpty() && !m_trajectorySegments.last().isEmpty()) {
        m_trajectorySegments.append(QVector<QPointF>());
    }
}
