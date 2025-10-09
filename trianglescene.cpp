#include "trianglescene.h"
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QTimer>
#include <QDebug>
#include <QPen>
#include <QMenu>
#include <QInputDialog>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <limits>

TriangleScene::TriangleScene(QObject* parent) : QGraphicsScene(parent)
{
    qDebug() << "TriangleScene constructor started";

    try {
        // Устанавливаем фиксированную область сцены
        setSceneRect(0, 0, 200, 200);

        // Создаем точки
        points.append(new DragPoint(50, 50, 8, 1.0));
        points.append(new DragPoint(100, 50, 8, 1.0));
        points.append(new DragPoint(75, 100, 8, 1.0));

        qDebug() << "Points created";

        // Добавляем точки к сцене и подключаем сигналы
        for (auto point : points) {
            if (point) {
                addItem(point);
                connect(point, &DragPoint::positionChanged, this, &TriangleScene::updateTriangle);
                connect(point, &DragPoint::positionChanging, this, &TriangleScene::updateTriangle);
                connect(point, &DragPoint::dragFinished, this, [this]() {
                    updateTriangle();
                    emit dragFinished();
                });

                point->setFlag(QGraphicsItem::ItemIsSelectable, true);
                point->setFlag(QGraphicsItem::ItemIsFocusable, true);
                point->setAcceptHoverEvents(true);
            }
        }

        // Создаем линии
        for (int i = 0; i < 3; ++i) {
            QGraphicsLineItem* line = new QGraphicsLineItem();
            if (line) {
                addItem(line);
                lines.append(line);
            }
        }

        qDebug() << "Lines created";

        // Создаем метки
        for (int i = 0; i < 3; ++i) {
            QGraphicsSimpleTextItem* label = new QGraphicsSimpleTextItem();
            if (label) {
                addItem(label);
                labels.append(label);
                label->setZValue(10);
                label->setBrush(QBrush(Qt::black));

                QFont font = label->font();
                font.setPointSize(8);
                label->setFont(font);
            }
        }

        qDebug() << "Labels created";

        // Инициализируем треугольник
        updateTriangle();

        qDebug() << "TriangleScene constructor completed";
    }
    catch (const std::exception& e) {
        qCritical() << "Exception in TriangleScene constructor: " << e.what();
    }
    catch (...) {
        qCritical() << "Unknown exception in TriangleScene constructor";
    }
}

TriangleScene::~TriangleScene()
{
    // Отключаем все сигналы сначала
    for (auto point : points) {
        if (point) {
            disconnect(point, &DragPoint::positionChanged, this, &TriangleScene::updateTriangle);
            disconnect(point, &DragPoint::dragFinished, this, &TriangleScene::updateTriangle);
        }
    }

    // Удаляем все объекты
    for (auto point : points) {
        if (point) delete point;
    }
    for (auto line : lines) {
        if (line) delete line;
    }
    for (auto label : labels) {
        if (label) delete label;
    }
}

void TriangleScene::updateTriangle()
{
    // Проверяем, что у нас есть достаточно элементов
    if (points.size() != 3 || lines.size() != 3 || labels.size() != 3) {
        qWarning() << "Invalid number of elements in updateTriangle";
        return;
    }

    try {
        for (int i = 0; i < 3; ++i) {
            if (!points[i] || !lines[i] || !labels[i]) {
                qWarning() << "Invalid pointers in updateTriangle at index" << i;
                continue;
            }

            QPointF p1 = points[i]->pos();
            QPointF p2 = points[(i+1)%3]->pos();

            // Обновляем линии треугольника
            lines[i]->setLine(QLineF(p1, p2));

            // Обновляем метки
            QString massStr;
            double mass = points[i]->mass();
            if (mass > 1000 || (mass < 0.001 && mass > 0)) {
                massStr = QString::number(mass, 'e', 3);
            } else {
                massStr = QString::number(mass, 'f', 3);
            }

            labels[i]->setText(QString("P%1: (%2, %3)\nM: %4")
                                   .arg(i+1)
                                   .arg(p1.x(), 0, 'f', 1)
                                   .arg(p1.y(), 0, 'f', 1)
                                   .arg(massStr));
            labels[i]->setPos(p1 + QPointF(10, -30));
        }

        // Убедимся, что сигнал испускается
        emit triangleUpdated();
    }
    catch (const std::exception& e) {
        qCritical() << "Exception in updateTriangle: " << e.what();
    }
    catch (...) {
        qCritical() << "Unknown exception in updateTriangle";
    }
}

void TriangleScene::setPoints(const QList<QPointF>& newPoints)
{
    if (newPoints.size() != 3) {
        qWarning() << "setPoints requires exactly 3 points, got" << newPoints.size();
        return;
    }

    // Проверяем указатели
    for (int i = 0; i < 3; ++i) {
        if (!points[i]) {
            qWarning() << "Invalid point pointer at index" << i;
            return;
        }
    }

    // Проверяем координаты
    for (int i = 0; i < 3; ++i) {
        if (std::isnan(newPoints[i].x()) || std::isinf(newPoints[i].x()) ||
            std::isnan(newPoints[i].y()) || std::isinf(newPoints[i].y())) {
            qWarning() << "Invalid point coordinates at index" << i << ":" << newPoints[i];
            return;
        }
    }

    // Временно блокируем сигналы
    bool wasBlocked = signalsBlocked();
    blockSignals(true);

    try {
        // Устанавливаем новые позиции
        for (int i = 0; i < 3; ++i) {
            DragPoint* point = dynamic_cast<DragPoint*>(points[i]);
            if (point) {
                point->setPosSilent(newPoints[i]);
            } else {
                points[i]->setPos(newPoints[i]);
            }
        }

        // Обновляем треугольник
        updateTriangle();
    }
    catch (const std::exception& e) {
        qWarning() << "Exception in setPoints:" << e.what();
    }
    catch (...) {
        qWarning() << "Unknown exception in setPoints";
    }

    // Восстанавливаем блокировку сигналов
    blockSignals(wasBlocked);
}

QList<QPointF> TriangleScene::getPoints() const
{
    QList<QPointF> result;
    for (int i = 0; i < 3; ++i) {
        if (i < points.size() && points[i]) {
            result.append(points[i]->pos());
        } else {
            qWarning() << "Invalid point access in getPoints at index" << i;
            result.append(QPointF(0, 0));
        }
    }
    return result;
}

QList<double> TriangleScene::getMasses() const
{
    QList<double> result;
    for (int i = 0; i < 3; ++i) {
        if (i < points.size() && points[i]) {
            result.append(points[i]->mass());
        } else {
            qWarning() << "Invalid point access in getMasses at index" << i;
            result.append(1.0);
        }
    }
    return result;
}

QRectF TriangleScene::itemsBoundingRect() const
{
    QRectF rect;
    for (auto point : points) {
        if (point) {
            rect = rect.united(point->sceneBoundingRect());
        }
    }
    return rect;
}

void TriangleScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // Если нажата левая кнопка и не на элементе - начинаем перетаскивание сцены
    if (event->button() == Qt::LeftButton && !itemAt(event->scenePos(), QTransform())) {
        isDraggingScene = true;
        lastDragPos = event->scenePos();
        emit sceneDragStarted();
        event->accept();
        return;
    }
    QGraphicsScene::mousePressEvent(event);
}

void TriangleScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (isDraggingScene) {
        QPointF delta = event->scenePos() - lastDragPos;
        lastDragPos = event->scenePos();

        // Перемещаем все точки
        for (auto point : points) {
            if (point) {
                point->setPos(point->pos() + delta);
            }
        }
        updateTriangle();
        // Немедленно отправляем сигнал об изменении
        emit triangleUpdated();
        event->accept();
        return;
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void TriangleScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isDraggingScene) {
        isDraggingScene = false;
        emit sceneDragFinished();
        emit dragFinished();
        event->accept();
        return;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

void TriangleScene::setMasses(const QList<double>& masses) {
    if (masses.size() != 3) {
        qWarning() << "setMasses requires exactly 3 masses, got" << masses.size();
        return;
    }

    // Проверяем указатели
    for (int i = 0; i < 3; ++i) {
        if (!points[i]) {
            qWarning() << "Invalid point pointer at index" << i;
            return;
        }
    }

    // Проверяем массы
    for (int i = 0; i < 3; ++i) {
        if (std::isnan(masses[i]) || std::isinf(masses[i]) || masses[i] <= 0) {
            qWarning() << "Invalid mass at index" << i << ":" << masses[i];
            return;
        }
    }

    // Временно блокируем сигналы
    bool wasBlocked = signalsBlocked();
    blockSignals(true);

    try {
        // Устанавливаем новые массы
        for (int i = 0; i < 3; ++i) {
            points[i]->setMass(masses[i]);
        }

        // Обновляем треугольник
        updateTriangle();
    }
    catch (const std::exception& e) {
        qWarning() << "Exception in setMasses:" << e.what();
    }
    catch (...) {
        qWarning() << "Unknown exception in setMasses";
    }

    // Восстанавливаем блокировку сигналов
    blockSignals(wasBlocked);
}

void TriangleScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
    DragPoint* point = dynamic_cast<DragPoint*>(item);

    if (point) {
        QMenu menu;
        QAction* editMassAction = menu.addAction("Edit Mass");

        QAction* selectedAction = menu.exec(event->screenPos());

        if (selectedAction == editMassAction) {
            bool ok;
            double mass = QInputDialog::getDouble(nullptr, "Edit Mass",
                                                  "Mass:", point->mass(),
                                                  -std::numeric_limits<double>::max(),
                                                  std::numeric_limits<double>::max(),
                                                  6, &ok);
            if (ok) {
                if (std::isnan(mass) || std::isinf(mass)) {
                    QMessageBox::warning(nullptr, "Invalid Mass",
                                         "Mass value is not a valid number.");
                    return;
                }

                if (mass <= 0) {
                    QMessageBox::warning(nullptr, "Invalid Mass",
                                         "Mass must be positive.");
                    return;
                }

                point->setMass(mass);
                updateTriangle();
            }
        }
    }

    QGraphicsScene::contextMenuEvent(event);
}
