#include "complexplaneview2.h"
#include "coordtransform.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QResizeEvent>
#include <QDebug>
#include <QGraphicsRectItem> // Для легенды

ComplexPlaneView2::ComplexPlaneView2(QWidget *parent)
    : QGraphicsView(parent), m_showTrajectory(false), m_drawingEnabled(true)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("QGraphicsView { border: 2px solid #8B4513; background-color: #FFF8DC; }");
    setDragMode(QGraphicsView::NoDrag);
    setMinimumSize(300, 300);

    // Устанавливаем сцену для расширенной области
    m_scene->setSceneRect(-3.2, -3.2, 6.4, 6.4);

    // Цвета для ветвей
    m_branchColors = {
        QColor(255, 0, 0),    // Красный
        QColor(0, 255, 0),    // Зеленый
        QColor(0, 0, 255),    // Синий
        QColor(255, 165, 0)   // Оранжевый
    };

    // Создаем систему координат
    createCoordinateSystem();

    // Создаем элементы для точек (максимум 4)
    for (int i = 0; i < 4; ++i) {
        QGraphicsEllipseItem* pointItem = new QGraphicsEllipseItem(-0.03, -0.03, 0.06, 0.06);
        pointItem->setBrush(QBrush(m_branchColors[i]));
        pointItem->setPen(QPen(Qt::black, 0.01));
        pointItem->setZValue(10);
        pointItem->setVisible(false);
        m_scene->addItem(pointItem);
        m_pointItems.append(pointItem);
    }

    // Создаем элементы для траекторий (по одному на ветвь)
    for (int i = 0; i < 4; ++i) {
        QGraphicsPathItem* trajectoryItem = new QGraphicsPathItem;
        trajectoryItem->setPen(QPen(m_branchColors[i], 0.015));
        trajectoryItem->setZValue(5);
        trajectoryItem->setVisible(false);
        m_scene->addItem(trajectoryItem);
        m_trajectoryItems.append(trajectoryItem);

        // Инициализируем траектории для каждой ветви
        m_trajectoryBranches[i].append(QVector<QPointF>());
    }

    // Центрируем и подгоняем вид
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void ComplexPlaneView2::createCoordinateSystem()
{
    // Создаем координатную сетку
    for (double value = -3.0; value <= 3.0; value += 0.5) {
        // Вертикальные линии сетки
        if (value != 0) {
            QGraphicsLineItem* gridLineX = new QGraphicsLineItem(value, -3.0, value, 3.0);
            gridLineX->setPen(QPen(QColor(240, 240, 240), 0.005));
            gridLineX->setZValue(0);
            m_scene->addItem(gridLineX);
        }

        // Горизонтальные линии сетки
        if (value != 0) {
            QGraphicsLineItem* gridLineY = new QGraphicsLineItem(-3.0, value, 3.0, value);
            gridLineY->setPen(QPen(QColor(240, 240, 240), 0.005));
            gridLineY->setZValue(0);
            m_scene->addItem(gridLineY);
        }
    }

    // Ось X
    QGraphicsLineItem* xAxis = new QGraphicsLineItem(-3.0, 0, 3.0, 0);
    xAxis->setPen(QPen(Qt::black, 0.02));
    xAxis->setZValue(1);
    m_scene->addItem(xAxis);

    // Ось Y
    QGraphicsLineItem* yAxis = new QGraphicsLineItem(0, -3.0, 0, 3.0);
    yAxis->setPen(QPen(Qt::black, 0.02));
    yAxis->setZValue(1);
    m_scene->addItem(yAxis);

    // Подписи осей
    QGraphicsSimpleTextItem* xLabel = new QGraphicsSimpleTextItem("Re(z)");
    xLabel->setPos(3.1, -0.3);
    QFont xFont = xLabel->font();
    xFont.setPointSize(12);
    xLabel->setFont(xFont);
    xLabel->setZValue(2);
    m_scene->addItem(xLabel);

    QGraphicsSimpleTextItem* yLabel = new QGraphicsSimpleTextItem("Im(z)");
    yLabel->setPos(-0.4, 3.1);
    QFont yFont = yLabel->font();
    yFont.setPointSize(12);
    yLabel->setFont(yFont);
    yLabel->setZValue(2);
    m_scene->addItem(yLabel);

    // Деления и подписи на оси X
    for (double value = -3.0; value <= 3.0; value += 1.0) {
        if (value == 0) continue;

        // Засечка
        QGraphicsLineItem* tick = new QGraphicsLineItem(value, -0.05, value, 0.05);
        tick->setPen(QPen(Qt::black, 0.015));
        tick->setZValue(1);
        m_scene->addItem(tick);
    }

    // Деления и подписи на оси Y
    for (double value = -3.0; value <= 3.0; value += 1.0) {
        if (value == 0) continue;

        // Засечка
        QGraphicsLineItem* tick = new QGraphicsLineItem(-0.05, value, 0.05, value);
        tick->setPen(QPen(Qt::black, 0.015));
        tick->setZValue(1);
        m_scene->addItem(tick);
    }

    // Легенда для ветвей
    QGraphicsSimpleTextItem* legendTitle = new QGraphicsSimpleTextItem(" ");
    legendTitle->setPos(-3.0, -2.8);
    QFont legendFont = legendTitle->font();
    legendFont.setPointSize(9);
    legendTitle->setFont(legendFont);
    legendTitle->setZValue(10);
    m_scene->addItem(legendTitle);

    for (int i = 0; i < 4; ++i) {
        // Цветные квадратики
        QGraphicsRectItem* colorRect = new QGraphicsRectItem(-2.9, -2.6 + i * 0.2, 0.1, 0.1);
        colorRect->setBrush(QBrush(m_branchColors[i]));
        colorRect->setPen(QPen(Qt::black, 0.005));
        colorRect->setZValue(10);
        m_scene->addItem(colorRect);
    }
}

void ComplexPlaneView2::setSolutions(const QVector<ComplexSolution>& solutions)
{
    m_currentSolutions = solutions;
    updatePoints();
}

void ComplexPlaneView2::addToTrajectory(const QVector<ComplexSolution>& solutions)
{
    if (!m_drawingEnabled) return;

    for (const ComplexSolution& solution : solutions) {
        int branch = solution.branch;
        if (branch >= 0 && branch < 4) {
            if (m_trajectoryBranches[branch].isEmpty()) {
                m_trajectoryBranches[branch].append(QVector<QPointF>());
            }

            m_trajectoryBranches[branch].last().append(solution.point);

            // Ограничиваем длину траектории
            if (m_trajectoryBranches[branch].last().size() > 500) {
                m_trajectoryBranches[branch].last().removeFirst();
            }
        }
    }

    updateTrajectory();
}

void ComplexPlaneView2::updatePoints()
{
    // Сначала скрываем все точки
    for (auto item : m_pointItems) {
        if (item) item->setVisible(false);
    }

    // Показываем только те точки, для которых есть решения
    for (int i = 0; i < m_currentSolutions.size() && i < m_pointItems.size(); ++i) {
        if (m_pointItems[i]) {
            QPointF scenePos = complexToScene(m_currentSolutions[i].point);
            m_pointItems[i]->setRect(scenePos.x() - 0.03, scenePos.y() - 0.03, 0.06, 0.06);
            m_pointItems[i]->setBrush(QBrush(m_currentSolutions[i].color));
            m_pointItems[i]->setVisible(true);
        }
    }
}

void ComplexPlaneView2::updateTrajectory()
{
    for (int branch = 0; branch < 4; ++branch) {
        if (!m_trajectoryItems[branch]) continue;

        QPainterPath path;
        bool hasPoints = false;

        for (const auto& segment : m_trajectoryBranches[branch]) {
            if (segment.size() < 2) continue;

            QPointF firstScenePos = complexToScene(segment.first());
            path.moveTo(firstScenePos.x(), firstScenePos.y());

            for (int i = 1; i < segment.size(); ++i) {
                QPointF scenePos = complexToScene(segment[i]);
                path.lineTo(scenePos.x(), scenePos.y());
            }

            hasPoints = true;
        }

        m_trajectoryItems[branch]->setPath(path);
        m_trajectoryItems[branch]->setVisible(m_showTrajectory && hasPoints);
    }
}

QPointF ComplexPlaneView2::sceneToComplex(const QPointF& scenePoint) const
{
    // Просто возвращаем координаты сцены (они уже в диапазоне [-3, 3])
    double re = qBound(m_minValue, scenePoint.x(), m_maxValue);
    double im = qBound(m_minValue, scenePoint.y(), m_maxValue);
    return QPointF(re, im);
}

QPointF ComplexPlaneView2::complexToScene(const QPointF& complexPoint) const
{
    // Просто возвращаем координаты (они уже в системе координат сцены)
    double x = qBound(m_minValue, complexPoint.x(), m_maxValue);
    double y = qBound(m_minValue, complexPoint.y(), m_maxValue);
    return QPointF(x, y);
}

void ComplexPlaneView2::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsView::drawForeground(painter, rect);
}

void ComplexPlaneView2::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    // Подгоняем вид при изменении размера
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void ComplexPlaneView2::clearTrajectory()
{
    for (int i = 0; i < 4; ++i) {
        m_trajectoryBranches[i].clear();
        m_trajectoryBranches[i].append(QVector<QPointF>());
    }
    updateTrajectory();
}

void ComplexPlaneView2::setShowTrajectory(bool show)
{
    m_showTrajectory = show;
    updateTrajectory();
}

void ComplexPlaneView2::breakTrajectory()
{
    // Создаем новый сегмент траектории для каждой ветви
    for (int i = 0; i < 4; ++i) {
        if (!m_trajectoryBranches[i].isEmpty() && !m_trajectoryBranches[i].last().isEmpty()) {
            m_trajectoryBranches[i].append(QVector<QPointF>());
        }
    }
}
