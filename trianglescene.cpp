#include "trianglescene.h"
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QTimer>
#include <QDebug>
#include <QPen>
#include <QMenu>
#include <QInputDialog>
#include <QGraphicsSceneContextMenuEvent>
#include <QMessageBox>
#include <limits>

TriangleScene::TriangleScene(QObject* parent) : QGraphicsScene(parent) {
    // Create points with initial positions and masses
    points.append(new DragPoint(100, 100, 15, 1.0));
    points.append(new DragPoint(200, 100, 15, 1.0));
    points.append(new DragPoint(150, 200, 15, 1.0));

    // Add points to scene and connect signals
    for (auto point : points) {
        addItem(point);
        connect(point, &DragPoint::positionChanged, this, &TriangleScene::updateTriangle);
        connect(point, &DragPoint::dragFinished, this, [this]() {
            updateTriangle();
            emit dragFinished(); // Испускаем сигнал при окончании перетаскивания
        });

        // Enable context menu for points
        point->setFlag(QGraphicsItem::ItemIsSelectable, true);
        point->setFlag(QGraphicsItem::ItemIsFocusable, true);
        point->setAcceptHoverEvents(true);
    }

    // Create lines and labels
    for (int i = 0; i < 3; ++i) {
        QGraphicsLineItem* line = new QGraphicsLineItem();
        addItem(line);
        lines.append(line);

        QGraphicsSimpleTextItem* label = new QGraphicsSimpleTextItem("");
        addItem(label);
        labels.append(label);
        label->setZValue(10);
        label->setBrush(QBrush(Qt::black));
    }

    // Initialize triangle
    updateTriangle();
}
TriangleScene::~TriangleScene() {
    // Disconnect all signals first
    for (auto point : points) {
        if (point) {
            disconnect(point, &DragPoint::positionChanged, this, &TriangleScene::updateTriangle);
            disconnect(point, &DragPoint::dragFinished, this, &TriangleScene::updateTriangle);
        }
    }

    // Delete all objects
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

void TriangleScene::updateTriangle() {
    // Check if all pointers are valid
    for (int i = 0; i < 3; ++i) {
        if (!points[i] || !lines[i] || !labels[i]) {
            qWarning() << "Invalid pointers in updateTriangle";
            return;
        }

        QPointF p1 = points[i]->pos();
        QPointF p2 = points[(i+1)%3]->pos();

        // Update triangle lines
        lines[i]->setLine(QLineF(p1, p2));

        // Update labels with coordinates and mass
        // Use scientific notation for very large or very small masses
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

    // Update scene area with some margin
    QRectF rect = itemsBoundingRect();
    if (rect.isValid() && !rect.isNull()) {
        // Ограничиваем максимальный размер сцены
        const qreal maxSize = 1000.0;
        if (rect.width() > maxSize || rect.height() > maxSize) {
            // Если сцена слишком большая, центрируем и устанавливаем разумный размер
            QPointF center = rect.center();
            rect = QRectF(center.x() - maxSize/2, center.y() - maxSize/2,
                          maxSize, maxSize);
        } else {
            rect.adjust(-50, -50, 50, 50);
        }
        setSceneRect(rect);
    }

    // Emit signals
    emit triangleUpdated();
    emit coordinatesUpdated(getPointCoordinates());
}

void TriangleScene::setPoints(const QList<QPointF>& newPoints) {
    if (newPoints.size() != 3) {
        qWarning() << "setPoints requires exactly 3 points, got" << newPoints.size();
        return;
    }

    // Check for NaN or Inf values
    for (int i = 0; i < 3; ++i) {
        if (std::isnan(newPoints[i].x()) || std::isinf(newPoints[i].x()) ||
            std::isnan(newPoints[i].y()) || std::isinf(newPoints[i].y())) {
            qWarning() << "Invalid point coordinates at index" << i << ":" << newPoints[i];
            return;
        }

        // Limit point coordinates to prevent extreme values
        if (std::abs(newPoints[i].x()) > 10000 || std::abs(newPoints[i].y()) > 10000) {
            qWarning() << "Point coordinates out of range at index" << i << ":" << newPoints[i];
            return;
        }
    }

    // Check if points are valid
    for (int i = 0; i < 3; ++i) {
        if (!points[i]) {
            qWarning() << "Invalid point pointer at index" << i;
            return;
        }
    }

    // Temporarily block signals to prevent recursive updates
    bool wasBlocked = signalsBlocked();
    blockSignals(true);

    try {
        // Set new positions using silent method to prevent unwanted signals
        for (int i = 0; i < 3; ++i) {
            // Cast to DragPoint and use the silent method if available
            DragPoint* point = dynamic_cast<DragPoint*>(points[i]);
            if (point) {
                point->setPosSilent(newPoints[i]);
            } else {
                points[i]->setPos(newPoints[i]);
            }
        }

        // Update the triangle
        updateTriangle();
    }
    catch (const std::exception& e) {
        qWarning() << "Exception in setPoints:" << e.what();
    }
    catch (...) {
        qWarning() << "Unknown exception in setPoints";
    }

    // Restore signal blocking state
    blockSignals(wasBlocked);
}

QList<QPointF> TriangleScene::getPoints() const {
    QList<QPointF> result;
    for (int i = 0; i < 3; ++i) {
        if (points[i]) {
            result.append(points[i]->pos());
        } else {
            result.append(QPointF(0, 0));
        }
    }
    return result;
}

QList<double> TriangleScene::getMasses() const {
    QList<double> result;
    for (int i = 0; i < 3; ++i) {
        if (points[i]) {
            result.append(points[i]->mass());
        } else {
            result.append(1.0);
        }
    }
    return result;
}

QString TriangleScene::getPointCoordinates() const {
    QString coords;
    for (int i = 0; i < 3; ++i) {
        QPointF p = points[i] ? points[i]->pos() : QPointF(0, 0);
        double m = points[i] ? points[i]->mass() : 1.0;

        // Use scientific notation for very large or very small masses
        QString massStr;
        if (m > 1000 || (m < 0.001 && m > 0)) {
            massStr = QString::number(m, 'e', 3);
        } else {
            massStr = QString::number(m, 'f', 3);
        }

        coords += QString("P%1: (%2, %3) M: %4\n")
                      .arg(i+1)
                      .arg(p.x(), 0, 'f', 1)
                      .arg(p.y(), 0, 'f', 1)
                      .arg(massStr);
    }
    return coords;
}

void TriangleScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
    DragPoint* point = dynamic_cast<DragPoint*>(item);

    if (point) {
        QMenu menu;
        QAction* editMassAction = menu.addAction("Edit Mass");

        QAction* selectedAction = menu.exec(event->screenPos());

        if (selectedAction == editMassAction) {
            bool ok;
            // Remove limits on mass values
            double mass = QInputDialog::getDouble(nullptr, "Edit Mass",
                                                  "Mass:", point->mass(),
                                                  -std::numeric_limits<double>::max(),
                                                  std::numeric_limits<double>::max(),
                                                  6, &ok);
            if (ok) {
                // Check for valid mass
                if (std::isnan(mass) || std::isinf(mass)) {
                    QMessageBox::warning(nullptr, "Invalid Mass",
                                         "Mass value is not a valid number.");
                    return;
                }

                // Check for negative mass
                if (mass < 0) {
                    QMessageBox::warning(nullptr, "Invalid Mass",
                                         "Mass cannot be negative.");
                    return;
                }

                // Check for zero mass
                if (mass == 0) {
                    QMessageBox::warning(nullptr, "Invalid Mass",
                                         "Mass cannot be zero.");
                    return;
                }

                point->setMass(mass);
                updateTriangle();
            }
        }
    }

    QGraphicsScene::contextMenuEvent(event);
}
