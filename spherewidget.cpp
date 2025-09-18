#include "spherewidget.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLShaderProgram>
#include <cmath>
#include <QDebug>
#include <QApplication>
#include <QOpenGLContext>

SphereWidget::SphereWidget(QWidget* parent)
    : QOpenGLWidget(parent), spherePoint(0, 0, 0), rotation(1, 0, 0, 0), distance(5.0f), isDraggingPoint(false) {
    setMinimumSize(400, 400);
    setFocusPolicy(Qt::StrongFocus);

    // Проверяем поддержку OpenGL
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(2, 1);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    setFormat(format);
}

SphereWidget::~SphereWidget() {
}

QVector3D SphereWidget::getPoint() const {
    return spherePoint;
}

void SphereWidget::initializeGL() {
    //qDebug() << "Initializing OpenGL context";

    if (!context()) {
        qCritical() << "No OpenGL context";
        return;
    }

    if (!context()->isValid()) {
        qCritical() << "Invalid OpenGL context";
        return;
    }

    // Инициализируем функции OpenGL (возвращает void в Qt6)
    initializeOpenGLFunctions();

    //qDebug() << "OpenGL initialized successfully";

    // Получаем и выводим версию OpenGL
    const GLubyte* version = glGetString(GL_VERSION);
    if (version) {
        //qDebug() << "OpenGL version:" << reinterpret_cast<const char*>(version);
    } else {
        //qDebug() << "Unable to get OpenGL version";
    }

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}

void SphereWidget::resizeGL(int w, int h) {
    if (w <= 0 || h <= 0) return;

    glViewport(0, 0, w, h);
    projection.setToIdentity();
    float aspect = static_cast<float>(w) / static_cast<float>(h ? h : 1);
    projection.perspective(45.0f, aspect, 0.1f, 100.0f);
}

void SphereWidget::paintGL() {
    if (!isValid()) {
        qCritical() << "OpenGL context is not valid in paintGL";
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelView.setToIdentity();
    modelView.translate(0, 0, -distance);
    modelView.rotate(rotation);

    // Use fixed pipeline for simplicity
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection.constData());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(modelView.constData());

    drawSphere();
    drawCoordinateSystem();
    drawPoint();
}

void SphereWidget::setPoint(const QVector3D& point) {
    spherePoint = point;
    update();
}

void SphereWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Handle left-click to set point on sphere
        QVector3D newPoint = getSpherePointFromMouse(event->pos());
        if (!newPoint.isNull()) {
            spherePoint = newPoint;
            isDraggingPoint = true;
            // Don't emit signal during dragging
            update();
        }
    }
    else if (event->button() == Qt::RightButton) {
        lastMousePos = event->pos();
    }
}

void SphereWidget::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::RightButton) {
        // Rotate sphere with right button
        int dx = event->position().x() - lastMousePos.x();
        int dy = event->position().y() - lastMousePos.y();

        // Normalize rotation speed
        float sensitivity = 0.5f;

        // Create rotation quaternions
        QQuaternion yaw = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), dx * sensitivity);
        QQuaternion pitch = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), dy * sensitivity);

        // Combine rotations
        rotation = yaw * pitch * rotation;
        rotation.normalize();

        lastMousePos = event->pos();
        update();
    }
    else if (isDraggingPoint && (event->buttons() & Qt::LeftButton)) {
        // Move point with left button drag
        QVector3D newPoint = getSpherePointFromMouse(event->pos());
        if (!newPoint.isNull()) {
            spherePoint = newPoint;
            // Don't emit signal during dragging
            update();
        }
    }
}

void SphereWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && isDraggingPoint) {
        isDraggingPoint = false;
        // Emit signal only when dragging is finished
        emit spherePointClicked(spherePoint);
    }
}

void SphereWidget::wheelEvent(QWheelEvent* event) {
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
        distance -= numDegrees.y() * 0.05f;
        distance = qMax(2.0f, qMin(distance, 20.0f));
        update();
    }
    event->accept();
}

QVector3D SphereWidget::getSpherePointFromMouse(const QPoint& mousePos) const {
    // Convert mouse position to normalized device coordinates
    float x = (2.0f * mousePos.x()) / width() - 1.0f;
    float y = 1.0f - (2.0f * mousePos.y()) / height();

    // Create ray from camera through mouse position
    QVector4D rayClip(x, y, -1.0f, 1.0f);
    QVector4D rayEye = projection.inverted() * rayClip;
    rayEye.setZ(-1.0f);
    rayEye.setW(0.0f);

    // Convert to world coordinates
    QMatrix4x4 modelViewInverted = modelView.inverted();
    QVector4D rayWorld = modelViewInverted * rayEye;
    QVector3D rayDir(rayWorld.x(), rayWorld.y(), rayWorld.z());
    rayDir.normalize();

    // Camera position in world coordinates
    QVector3D cameraPos = QVector3D(modelViewInverted * QVector4D(0, 0, 0, 1));

    // Sphere parameters
    QVector3D sphereCenter(0, 0, 0);
    float sphereRadius = 1.0f;

    // Ray-sphere intersection
    QVector3D oc = cameraPos - sphereCenter;
    float a = QVector3D::dotProduct(rayDir, rayDir);
    float b = 2.0f * QVector3D::dotProduct(oc, rayDir);
    float c = QVector3D::dotProduct(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return QVector3D(0, 0, 0); // No intersection
    }

    float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
    QVector3D intersection = cameraPos + t * rayDir;

    return intersection.normalized();
}

void SphereWidget::drawSphere() {
    // Draw a more detailed sphere with latitude and longitude lines
    const int segments = 36;
    const int rings = 18;
    const float radius = 1.0f;

    // Draw latitude lines (parallels)
    glColor3f(0.6f, 0.6f, 0.6f);
    for (int i = 0; i <= rings; i++) {
        float phi = static_cast<float>(i) * M_PI / static_cast<float>(rings);
        glBegin(GL_LINE_LOOP);
        for (int j = 0; j <= segments; j++) {
            float theta = static_cast<float>(j) * 2.0f * M_PI / static_cast<float>(segments);
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            glVertex3f(x, y, z);
        }
        glEnd();
    }

    // Draw longitude lines (meridians)
    for (int i = 0; i <= segments; i++) {
        float theta = static_cast<float>(i) * 2.0f * M_PI / static_cast<float>(segments);
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j <= rings; j++) {
            float phi = static_cast<float>(j) * M_PI / static_cast<float>(rings);
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
}

void SphereWidget::drawCoordinateSystem() {
    glLineWidth(2.0f);
    glBegin(GL_LINES);

    // X axis (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(1.5f, 0, 0);

    // Y axis (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1.5f, 0);

    // Z axis (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1.5f);

    glEnd();
    glLineWidth(1.0f);
}

void SphereWidget::drawPoint() {
    if (spherePoint.isNull()) return;

    glColor3f(1.0f, 0.0f, 0.0f);
    glPointSize(8.0f);
    glBegin(GL_POINTS);
    glVertex3f(spherePoint.x(), spherePoint.y(), spherePoint.z());
    glEnd();

    // Draw a line from center to point
    glColor3f(1.0f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(spherePoint.x(), spherePoint.y(), spherePoint.z());
    glEnd();
}
