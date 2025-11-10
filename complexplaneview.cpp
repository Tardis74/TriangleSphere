#include "complexplaneview.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QResizeEvent>
#include <QDebug>

ComplexPlaneView::ComplexPlaneView(QWidget *parent)
    : QGraphicsView(parent), m_showTrajectory(false), m_drawingEnabled(true),
    m_pointItem(nullptr), m_trajectoryItem(nullptr)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("QGraphicsView { border: 2px solid #aaa; background-color: white; }");
    setDragMode(QGraphicsView::NoDrag);
    setMinimumSize(300, 300);

    // Устанавливаем сцену для области [-1.2, 1.2] × [-1.2, 1.2] (поля 20%)
    m_scene->setSceneRect(-1.2, -1.2, 2.4, 2.4);

    // Создаем систему координат как графические элементы сцены
    createCoordinateSystem();

    // Создаем элемент для точки
    m_pointItem = new QGraphicsEllipseItem(-0.03, -0.03, 0.06, 0.06);
    m_pointItem->setBrush(QBrush(Qt::red));
    m_pointItem->setPen(QPen(Qt::black, 0.01));
    m_pointItem->setZValue(10);
    m_scene->addItem(m_pointItem);
    m_pointItem->setVisible(false);

    // Создаем элемент для траектории
    m_trajectoryItem = new QGraphicsPathItem;
    m_trajectoryItem->setPen(QPen(QColor(0, 100, 200), 0.02));
    m_trajectoryItem->setZValue(5);
    m_scene->addItem(m_trajectoryItem);
    m_trajectoryItem->setVisible(false);

    m_trajectorySegments.append(QVector<QPointF>());

    // Центрируем и подгоняем вид
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void ComplexPlaneView::createCoordinateSystem()
{
    // Сначала создаем координатную сетку (светло-серая)
    for (double value = -1.0; value <= 1.0; value += 0.1) {
        // Вертикальные линии сетки
        if (value != 0) { // Пропускаем ось Y
            QGraphicsLineItem* gridLineX = new QGraphicsLineItem(value, -1.0, value, 1.0);
            gridLineX->setPen(QPen(QColor(220, 220, 220), 0.005)); // Светло-серый
            gridLineX->setZValue(0);
            m_scene->addItem(gridLineX);
        }

        // Горизонтальные линии сетки
        if (value != 0) { // Пропускаем ось X
            QGraphicsLineItem* gridLineY = new QGraphicsLineItem(-1.0, value, 1.0, value);
            gridLineY->setPen(QPen(QColor(220, 220, 220), 0.005)); // Светло-серый
            gridLineY->setZValue(0);
            m_scene->addItem(gridLineY);
        }
    }

    // Ось X (ξ₂)
    QGraphicsLineItem* xAxis = new QGraphicsLineItem(-1.0, 0, 1.0, 0);
    xAxis->setPen(QPen(Qt::black, 0.02));
    xAxis->setZValue(1);
    m_scene->addItem(xAxis);

    // Ось Y (ξ₃)
    QGraphicsLineItem* yAxis = new QGraphicsLineItem(0, -1.0, 0, 1.0);
    yAxis->setPen(QPen(Qt::black, 0.02));
    yAxis->setZValue(1);
    m_scene->addItem(yAxis);

    // Подписи осей
    QGraphicsSimpleTextItem* xLabel = new QGraphicsSimpleTextItem("ξ₂");
    xLabel->setPos(1.05, -0.1);
    QFont xFont = xLabel->font();
    xFont.setPointSize(14);
    xLabel->setFont(xFont);
    xLabel->setZValue(2);
    m_scene->addItem(xLabel);

    QGraphicsSimpleTextItem* yLabel = new QGraphicsSimpleTextItem("ξ₃");
    yLabel->setPos(-0.1, 1.05);
    QFont yFont = yLabel->font();
    yFont.setPointSize(14);
    yLabel->setFont(yFont);
    yLabel->setZValue(2);
    m_scene->addItem(yLabel);

    // Деления и подписи на оси X (только основные -0.5, 0.5)
    for (double value = -1.0; value <= 1.0; value += 0.5) {
        if (value == 0) continue;

        // Засечка
        QGraphicsLineItem* tick = new QGraphicsLineItem(value, -0.03, value, 0.03);
        tick->setPen(QPen(Qt::black, 0.015));
        tick->setZValue(1);
        m_scene->addItem(tick);

        // Подпись
        QGraphicsSimpleTextItem* label = new QGraphicsSimpleTextItem(QString::number(value, 'f', 1));
        label->setPos(value - 0.05, 0.06);
        QFont font = label->font();
        font.setPointSize(10);
        label->setFont(font);
        label->setZValue(2);
        m_scene->addItem(label);
    }

    // Деления и подписи на оси Y (только основные -0.5, 0.5)
    for (double value = -1.0; value <= 1.0; value += 0.5) {
        if (value == 0) continue;

        // Засечка
        QGraphicsLineItem* tick = new QGraphicsLineItem(-0.03, value, 0.03, value);
        tick->setPen(QPen(Qt::black, 0.015));
        tick->setZValue(1);
        m_scene->addItem(tick);

        // Подпись
        QGraphicsSimpleTextItem* label = new QGraphicsSimpleTextItem(QString::number(value, 'f', 1));
        label->setPos(0.06, value - 0.05);
        QFont font = label->font();
        font.setPointSize(10);
        label->setFont(font);
        label->setZValue(2);
        m_scene->addItem(label);
    }

    // Подпись начала координат
    QGraphicsSimpleTextItem* origin = new QGraphicsSimpleTextItem("0");
    origin->setPos(0.03, 0.03);
    QFont originFont = origin->font();
    originFont.setPointSize(10);
    origin->setFont(originFont);
    origin->setZValue(2);
    m_scene->addItem(origin);
}

QPointF ComplexPlaneView::sceneToComplex(const QPointF& scenePoint) const
{
    // Просто возвращаем координаты сцены (они уже в диапазоне [-1, 1])
    // Ограничиваем значения
    double xi2 = qBound(-1.0, scenePoint.x(), 1.0);
    double xi3 = qBound(-1.0, scenePoint.y(), 1.0);
    return QPointF(xi2, xi3);
}

QPointF ComplexPlaneView::complexToScene(const QPointF& complexPoint) const
{
    // Просто возвращаем координаты (они уже в системе координат сцены)
    // Ограничиваем значения
    double x = qBound(-1.0, complexPoint.x(), 1.0);
    double y = qBound(-1.0, complexPoint.y(), 1.0);
    return QPointF(x, y);
}

void ComplexPlaneView::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsView::drawForeground(painter, rect);
    // Больше не рисуем здесь систему координат - она создана как элементы сцены
}

void ComplexPlaneView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    // Подгоняем вид при изменении размера
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void ComplexPlaneView::setPoint(const QPointF& point)
{
    m_currentPoint = point;
    updatePoint();
}

void ComplexPlaneView::updatePoint()
{
    if (!m_pointItem) return;

    if (m_currentPoint.isNull()) {
        m_pointItem->setVisible(false);
        return;
    }

    // Ограничиваем значения [-1, 1]
    double xi2 = qBound(m_minValue, m_currentPoint.x(), m_maxValue);
    double xi3 = qBound(m_minValue, m_currentPoint.y(), m_maxValue);

    // Преобразуем в координаты сцены
    QPointF scenePos = complexToScene(QPointF(xi2, xi3));

    m_pointItem->setRect(scenePos.x() - 0.03, scenePos.y() - 0.03, 0.06, 0.06);
    m_pointItem->setVisible(true);
}

void ComplexPlaneView::addToTrajectory(const QPointF& point)
{
    if (!m_drawingEnabled) return;

    if (m_trajectorySegments.isEmpty()) {
        m_trajectorySegments.append(QVector<QPointF>());
    }

    // Ограничиваем значения [-1, 1]
    double xi2 = qBound(m_minValue, point.x(), m_maxValue);
    double xi3 = qBound(m_minValue, point.y(), m_maxValue);

    m_trajectorySegments.last().append(QPointF(xi2, xi3));

    // Ограничиваем длину для производительности
    if (m_trajectorySegments.last().size() > 1000) {
        m_trajectorySegments.last().removeFirst();
    }

    updateTrajectory();
}

void ComplexPlaneView::updateTrajectory()
{
    if (!m_trajectoryItem) return;

    QPainterPath path;

    for (const auto& segment : m_trajectorySegments) {
        if (segment.size() < 2) continue;

        // Начинаем путь с первой точки
        QPointF firstScenePos = complexToScene(segment.first());
        path.moveTo(firstScenePos.x(), firstScenePos.y());

        // Добавляем линии к остальным точкам
        for (int i = 1; i < segment.size(); ++i) {
            QPointF scenePos = complexToScene(segment[i]);
            path.lineTo(scenePos.x(), scenePos.y());
        }
    }

    m_trajectoryItem->setPath(path);
    m_trajectoryItem->setVisible(m_showTrajectory && !path.isEmpty());
}

void ComplexPlaneView::clearTrajectory()
{
    m_trajectorySegments.clear();
    m_trajectorySegments.append(QVector<QPointF>());
    updateTrajectory();
}

void ComplexPlaneView::setShowTrajectory(bool show)
{
    m_showTrajectory = show;
    updateTrajectory();
}

void ComplexPlaneView::breakTrajectory()
{
    if (!m_trajectorySegments.isEmpty() && !m_trajectorySegments.last().isEmpty()) {
        m_trajectorySegments.append(QVector<QPointF>());
    }
}
