#include "spherewidget.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLShaderProgram>
#include <cmath>
#include <QDebug>
#include <QApplication>
#include <QOpenGLContext>
#include <QGraphicsOpacityEffect>

SphereWidget::SphereWidget(QWidget* parent)
    : QOpenGLWidget(parent), m_sphereRadius(1.0), spherePoint(0, 0, 1), rotation(1, 0, 0, 0),
    m_masses({1.0, 1.0, 1.0}), distance(5.0f), isDraggingPoint(false),
    isRotatingSphere(false), m_showTrajectory(false)
{
    m_trajectorySegments.append(QVector<QVector3D>());
    m_trajectorySegmentColors.append(QVector<QVector3D>());
    setMinimumSize(400, 400);
    setFocusPolicy(Qt::StrongFocus);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(2, 1);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setSamples(4);
    setFormat(format);
}

SphereWidget::~SphereWidget() {
}

QVector3D SphereWidget::getPoint() const {
    return spherePoint;
}

void SphereWidget::rotateToPoint(const QVector3D& point) {
    if (point.isNull()) return;

    // Вычисляем вращение, чтобы точка была направлена к камере
    QVector3D targetDir = point.normalized();
    QVector3D currentDir = QVector3D(0, 0, 1); // Направление камеры по умолчанию

    // Вычисляем ось и угол вращения
    QVector3D axis = QVector3D::crossProduct(currentDir, targetDir);
    if (axis.length() < 1e-6) {
        axis = QVector3D(0, 1, 0); // Если направления совпадают, используем ось Y
    }
    axis.normalize();

    float angle = std::acos(QVector3D::dotProduct(currentDir, targetDir)) * 180.0f / M_PI;

    // Применяем вращение
    rotation = QQuaternion::fromAxisAndAngle(axis, angle);
    update();
}

void SphereWidget::initializeGL() {
    if (!context()) {
        qCritical() << "No OpenGL context";
        return;
    }

    if (!context()->isValid()) {
        qCritical() << "Invalid OpenGL context";
        return;
    }

    initializeOpenGLFunctions();

    // Настройка освещения
    setupLighting();

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SphereWidget::setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Позиция света (правеer и выше наблюдателя)
    GLfloat lightPosition[] = {1.0f, 1.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    GLfloat ambientLight[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat diffuseLight[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat specularLight[] = {0.5f, 0.5f, 0.5f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
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

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection.constData());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(modelView.constData());

    // Сначала рисуем непрозрачные элементы
    glDisable(GL_LIGHTING);
    drawSpecialLines();
    drawCollisionPoints();
    drawEquilateralPoints();
    drawPoles();
    drawCoordinateSystem();

    // РИСУЕМ ТРАЕКТОРИЮ ДО ТОЧКИ (чтобы она была под точкой)
    drawTrajectory();

    if (m_showComplexPlane) {
        drawComplexPlane();
    }

    // Затем рисуем точку
    drawPoint();

    // РИСУЕМ ИНФОРМАЦИЮ О РАДИУСЕ
    drawRadiusInfo();

    glEnable(GL_LIGHTING);

    // Затем рисуем прозрачную сферу
    drawSphere();
}

void SphereWidget::setPoint(const QVector3D& point) {
    spherePoint = point;
    update();
}

void SphereWidget::mousePressEvent(QMouseEvent* event) {
    if (rotationMode) {
        // Режим вращения сферы - только левая кнопка
        if (event->button() == Qt::LeftButton) {
            isRotatingSphere = true;
            lastMousePos = event->pos();
        }
    } else {
        // Режим перемещения точки - левая кнопка создаёт/перемещает точку в любом месте
        if (event->button() == Qt::LeftButton) {
            isDraggingPoint = true;
            QVector3D newPoint = getSpherePointFromMouse(event->pos());
            if (!newPoint.isNull()) {
                spherePoint = newPoint;
                update();
                // Немедленно отправляем сигнал об изменении
                emit spherePointClicked(spherePoint);
            }
        }
    }
}

void SphereWidget::mouseMoveEvent(QMouseEvent* event) {
    if (rotationMode) {
        // Режим вращения сферы
        if (isRotatingSphere && (event->buttons() & Qt::LeftButton)) {
            int dx = event->pos().x() - lastMousePos.x();
            int dy = event->pos().y() - lastMousePos.y();

            float sensitivity = 0.5f;
            QQuaternion yaw = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), dx * sensitivity);
            QQuaternion pitch = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), dy * sensitivity);

            rotation = yaw * pitch * rotation;
            rotation.normalize();

            lastMousePos = event->pos();
            update();
        }
    } else {
        // Режим перемещения точки
        if (isDraggingPoint && (event->buttons() & Qt::LeftButton)) {
            QVector3D newPoint = getSpherePointFromMouse(event->pos());
            if (!newPoint.isNull()) {
                spherePoint = newPoint;
                update();
                // Немедленно отправляем сигнал об изменении
                emit spherePointClicked(spherePoint);
            }
        }
    }
}

void SphereWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (isDraggingPoint) {
            isDraggingPoint = false;
            if (!rotationMode) {
                emit spherePointClicked(spherePoint);
            }
        }
        if (isRotatingSphere) {
            isRotatingSphere = false;
        }
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

QVector3D SphereWidget::projectToScreen(const QVector3D& point) const {
    QVector4D pointHomogeneous(point, 1.0f);
    QVector4D projected = projection * modelView * pointHomogeneous;
    if (projected.w() == 0.0f) return QVector3D();
    projected /= projected.w();

    float x = (projected.x() + 1.0f) * 0.5f * width();
    float y = (1.0f - (projected.y() + 1.0f) * 0.5f) * height();
    return QVector3D(x, y, projected.z());
}

void SphereWidget::drawSphere() {
    const int segments = 36;
    const int rings = 18;
    const float radius = 1.0f;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Используем glColorMaterial для упрощения управления цветом
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Устанавливаем серый цвет с прозрачностью
    glColor4f(0.7f, 0.7f, 0.7f, 0.75f);

    // Отключаем specular для устранения цветных бликов
    GLfloat mat_specular[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat mat_shininess[] = {0.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    for (int i = 0; i < rings; i++) {
        float phi1 = static_cast<float>(i) * M_PI / static_cast<float>(rings);
        float phi2 = static_cast<float>(i + 1) * M_PI / static_cast<float>(rings);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; j++) {
            float theta = static_cast<float>(j) * 2.0f * M_PI / static_cast<float>(segments);

            for (int k = 0; k < 2; k++) {
                float phi = (k == 0) ? phi1 : phi2;

                float x = radius * sin(phi) * cos(theta);
                float y = radius * cos(phi);
                float z = radius * sin(phi) * sin(theta);

                glNormal3f(x, y, z);
                glVertex3f(x, y, z);
            }
        }
        glEnd();
    }

    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
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
    glLineWidth(2.0f);
}

void SphereWidget::drawPoint()
{
    if (spherePoint.isNull()) return;

    // Сохраняем текущие настройки освещения
    GLboolean lightingWasEnabled = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);

    // Ярко-красная точка - стандартный размер
    glColor3f(1.0f, 0.0f, 0.0f);
    glPointSize(10.0f);
    glBegin(GL_POINTS);
    glVertex3f(spherePoint.x(), spherePoint.y(), spherePoint.z());
    glEnd();

    // Белый контур для лучшей видимости
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(12.0f);
    glEnable(GL_POINT_SMOOTH);
    glBegin(GL_POINTS);
    glVertex3f(spherePoint.x(), spherePoint.y(), spherePoint.z());
    glEnd();
    glDisable(GL_POINT_SMOOTH);

    // Линия от центра к точке
    glColor3f(1.0f, 0.7f, 0.7f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(spherePoint.x(), spherePoint.y(), spherePoint.z());
    glEnd();

    // Восстанавливаем настройки
    if (lightingWasEnabled) {
        glEnable(GL_LIGHTING);
    }
}


void SphereWidget::drawEquilateralPoints()
{
    if (m_masses.size() != 3) return;

    double m1 = m_masses[0];
    double m2 = m_masses[1];
    double m3 = m_masses[2];

    // Вычисляем параметры
    double denominator1 = m1 + m2;
    double denominator2 = m1 + m2 + m3;
    if (denominator1 <= 0 || denominator2 <= 0) return;

    double mu1 = (m1 * m2) / denominator1;
    double mu2 = (m3 * denominator1) / denominator2;

    double A = m1 / denominator1;
    double B = m2 / denominator1;
    double AB = A * B;

    // Вычисляем координаты ξ (без множителя a²)
    double xi1 = mu1 - mu2 * (1.0 - AB);
    double xi2 = std::sqrt(mu1 * mu2) * (A - B);
    double xi3_pos = std::sqrt(mu1 * mu2) * std::sqrt(3.0);
    double xi3_neg = -xi3_pos;

    // Отключаем освещение для точек
    glDisable(GL_LIGHTING);

    // Нормализуем для положительного решения
    double norm_pos = std::sqrt(xi1*xi1 + xi2*xi2 + xi3_pos*xi3_pos);
    if (norm_pos > 1e-10) {
        double x = xi2 / norm_pos;
        double y = xi3_pos / norm_pos;
        double z = xi1 / norm_pos;

        // Ярко-желтая точка - тот же размер, что и основная точка
        glColor3f(1.0f, 1.0f, 0.0f);
        glPointSize(10.0f);
        glBegin(GL_POINTS);
        glVertex3f(x, y, z);
        glEnd();

        // Белый контур
        glColor3f(1.0f, 1.0f, 1.0f);
        glPointSize(12.0f);
        glEnable(GL_POINT_SMOOTH);
        glBegin(GL_POINTS);
        glVertex3f(x, y, z);
        glEnd();
        glDisable(GL_POINT_SMOOTH);
    }

    // Нормализуем для отрицательного решения
    double norm_neg = std::sqrt(xi1*xi1 + xi2*xi2 + xi3_neg*xi3_neg);
    if (norm_neg > 1e-10) {
        double x = xi2 / norm_neg;
        double y = xi3_neg / norm_neg;
        double z = xi1 / norm_neg;

        // Ярко-желтая точка - тот же размер, что и основная точка
        glColor3f(1.0f, 1.0f, 0.0f);
        glPointSize(10.0f);
        glBegin(GL_POINTS);
        glVertex3f(x, y, z);
        glEnd();

        // Белый контур
        glColor3f(1.0f, 1.0f, 1.0f);
        glPointSize(12.0f);
        glEnable(GL_POINT_SMOOTH);
        glBegin(GL_POINTS);
        glVertex3f(x, y, z);
        glEnd();
        glDisable(GL_POINT_SMOOTH);
    }

    // Включаем освещение обратно
    glEnable(GL_LIGHTING);
}

void SphereWidget::drawSpecialLines() {
    const int resolution = 100;

    // ЭКВАТОР - линия вырожденных треугольников (y = 0)
    glColor3f(1.0f, 1.0f, 0.0f); // Желтый цвет
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= resolution; i++) {
        float theta = static_cast<float>(i) * 2.0f * M_PI / static_cast<float>(resolution);
        float x = cos(theta);
        float y = 0.0f;  // Экватор: y = 0
        float z = sin(theta);
        glVertex3f(x, y, z);
    }
    glEnd();

    double m1 = m_masses[0];
    double m2 = m_masses[1];
    double m3 = m_masses[2];

    // Calculate reduced masses
    double denominator1 = m1 + m2;
    double denominator2 = m1 + m2 + m3;

    if (denominator1 <= 0 || denominator2 <= 0) return;

    double mu1 = (m1 * m2) / denominator1;
    double mu2 = (m3 * (m1 + m2)) / denominator2;

    // Calculate k parameter
    double k = (m1 / denominator1) * std::sqrt(mu2 / mu1);
    double k2 = k * k;
    double denom = 1.0 + k2;

    // Condition 1: Right angle at vertex r2 (xi2 = k*(1+xi1))
    glColor3f(0.0f, 1.0f, 1.0f); // Cyan
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= resolution; i++) {
        double theta = 2.0 * M_PI * i / resolution;
        double sin_theta = std::sin(theta);
        double cos_theta = std::cos(theta);

        double x = k / denom * (1.0 + sin_theta);
        double y = cos_theta / std::sqrt(denom);
        double z = (-k2 + sin_theta) / denom;

        glVertex3f(x, y, z);
    }
    glEnd();

    // Condition 2: Right angle at vertex r1 (xi2 = -k*(1+xi1))
    // Отражение относительно плоскости x = -z
    glColor3f(0.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= resolution; i++) {
        double theta = 2.0 * M_PI * i / resolution;
        double sin_theta = std::sin(theta);
        double cos_theta = std::cos(theta);

        // Оригинальные координаты (как в условии 1)
        double x_orig = k / denom * (1.0 + sin_theta);
        double y_orig = cos_theta / std::sqrt(denom);
        double z_orig = (-k2 + sin_theta) / denom;

        // Отражение относительно плоскости x = -z
        double x = -x_orig;
        double y = y_orig;
        double z = z_orig;

        glVertex3f(x, y, z);
    }
    glEnd();

    // Condition 3: Right angle at vertex r3 (xi2^2 + xi3^2 = k^2*(1+xi1)^2)
    glColor3f(0.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= resolution; i++) {
        double theta = 2.0 * M_PI * i / resolution;

        // Это окружность на постоянной широте
        double phi0 = 2.0 * std::atan(k);
        double x = std::sin(phi0) * std::cos(theta);
        double y = std::sin(phi0) * std::sin(theta);
        double z = std::cos(phi0);

        glVertex3f(x, y, z);
    }
    glEnd();
}

void SphereWidget::setMasses(const QList<double>& masses)
{
    qDebug() << "SphereWidget::setMasses called with masses:" << masses;
    if (masses.size() != 3) {
        qWarning() << "SphereWidget::setMasses requires exactly 3 masses";
        return;
    }

    // Проверяем валидность масс
    bool valid = true;
    for (int i = 0; i < 3; ++i) {
        if (std::isnan(masses[i]) || std::isinf(masses[i]) || masses[i] <= 0) {
            qWarning() << "Invalid mass at index" << i << ":" << masses[i];
            valid = false;
            break;
        }
    }

    if (valid) {
        m_masses = masses;
        update(); // Перерисовываем сцену с новыми точками соударения
    }
}

void SphereWidget::drawCollisionPoints() {
    // Три точки соударений
    double m1 = m_masses[0];
    double m2 = m_masses[1];
    double m3 = m_masses[2];

    glColor3f(1.0f, 1.0f, 1.0f); // Белый цвет
    glPointSize(8.0f);
    glBegin(GL_POINTS);

    glVertex3d(0.0, 0.0, -1.0);
    glVertex3d(-2*sqrt(m1*m2*m3*(m1+m2+m3))/((m1+m2)*(m1+m3)), 0,-(m2*m3-m1*(m1+m2+m3))/((m1+m2)*(m1+m3)));
    glVertex3d(2*sqrt(m1*m2*m3*(m1+m2+m3))/((m1+m2)*(m1+m3)), 0,-(m2*m3-m1*(m1+m2+m3))/((m1+m2)*(m1+m3)));

    glEnd();
    glPointSize(1.0f);
}

void SphereWidget::drawPoles() {
    // Северный и южный полюса
    glDisable(GL_LIGHTING);

    glColor3f(1.0f, 1.0f, 0.0f); // Желтый цвет
    glPointSize(10.0f);
    glBegin(GL_POINTS);
    glVertex3f(0.0f, 1.0f, 0.0f);   // Северный полюс
    glVertex3f(0.0f, -1.0f, 0.0f);  // Южный полюс
    glEnd();
    glPointSize(1.0f);

    glEnable(GL_LIGHTING);
}

void SphereWidget::setShowTrajectory(bool show)
{
    m_showTrajectory = show;
    update();
}

void SphereWidget::clearTrajectory()
{
    m_trajectorySegments.clear();
    m_trajectorySegmentColors.clear();
    // Создаем первый пустой сегмент
    m_trajectorySegments.append(QVector<QVector3D>());
    m_trajectorySegmentColors.append(QVector<QVector3D>());
    update();
}

void SphereWidget::addToTrajectory(const QVector3D& point)
{
    if (!m_drawingEnabled) return;

    if (m_trajectorySegments.isEmpty()) {
        m_trajectorySegments.append(QVector<QVector3D>());
        m_trajectorySegmentColors.append(QVector<QVector3D>());
    }

    m_trajectorySegments.last().append(point);

    // Яркий бирюзовый цвет для траектории (лучшая видимость)
    float t = static_cast<float>(m_trajectorySegments.last().size()) / 1000.0f;
    QVector3D color(0.0f, 0.8f + 0.2f * t, 1.0f - 0.5f * t); // От бирюзового к синему
    m_trajectorySegmentColors.last().append(color);

    // Ограничиваем длину каждого сегмента для производительности
    if (m_trajectorySegments.last().size() > 1000) {
        m_trajectorySegments.last().removeFirst();
        m_trajectorySegmentColors.last().removeFirst();
    }

    update();
}

void SphereWidget::drawTrajectory()
{
    if (!m_showTrajectory || m_trajectorySegments.isEmpty()) {
        return;
    }

    glDisable(GL_LIGHTING);

    // Увеличим толщину линии траектории для лучшей видимости
    glLineWidth(4.0f);

    // Рисуем каждый сегмент отдельно
    for (int seg = 0; seg < m_trajectorySegments.size(); ++seg) {
        const auto& segment = m_trajectorySegments[seg];

        if (segment.size() < 2) continue;

        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < segment.size(); ++i) {
            // Используем градиент цвета
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex3f(segment[i].x(), segment[i].y(), segment[i].z());
        }
        glEnd();
    }

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

void SphereWidget::breakTrajectory()
{
    // Создаем новый сегмент траектории
    if (!m_trajectorySegments.isEmpty() && !m_trajectorySegments.last().isEmpty()) {
        m_trajectorySegments.append(QVector<QVector3D>());
        m_trajectorySegmentColors.append(QVector<QVector3D>());
    }
}

// Новый метод для установки радиуса
void SphereWidget::setSphereRadius(double radius) {
    if (radius > 0 && !std::isnan(radius) && !std::isinf(radius)) {
        m_sphereRadius = radius;
        update();
    }
}

void SphereWidget::setShowComplexPlane(bool show) {
    m_showComplexPlane = show;
    update();
}

void SphereWidget::setComplexPlanePoint(const QVector3D& rawCoords) {
    m_complexPlanePoint = rawCoords;
    if (m_showComplexPlane) {
        update();
    }
}
void SphereWidget::drawComplexPlane() {
    glDisable(GL_LIGHTING);

    // Рисуем плоскость (диск) в плоскости XZ (xi2-xi3)
    glColor4f(0.2f, 0.4f, 0.8f, 0.3f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, 0); // Центр

    const int segments = 36;
    float radius = 1.5f; // Размер плоскости

    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        glVertex3f(x, 0, z);
    }
    glEnd();

    // Рисуем контур плоскости
    glColor4f(0.1f, 0.2f, 0.6f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        glVertex3f(x, 0, z);
    }
    glEnd();

    // Рисуем оси комплексной плоскости
    glColor3f(1.0f, 1.0f, 0.0f); // Желтый цвет для осей
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    // Ось X (xi2)
    glVertex3f(-radius, 0, 0);
    glVertex3f(radius, 0, 0);
    // Ось Z (xi3)
    glVertex3f(0, 0, -radius);
    glVertex3f(0, 0, radius);
    glEnd();

    // Подписи осей
    // (В OpenGL без шейдеров текст сложно рисовать, поэтому пропускаем)

    // Рисуем проекцию точки на комплексную плоскость
    if (!m_complexPlanePoint.isNull()) {
        // Нормализуем координаты для отображения на плоскости радиуса 1.5
        QVector3D normalized = m_complexPlanePoint.normalized() * 1.5f;

        glColor3f(1.0f, 0.0f, 1.0f); // Пурпурный цвет для проекции
        glPointSize(8.0f);
        glBegin(GL_POINTS);
        glVertex3f(normalized.x(), 0, normalized.z());
        glEnd();

        // Линия от центра к проекции
        glColor3f(1.0f, 0.5f, 1.0f);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(normalized.x(), 0, normalized.z());
        glEnd();
    }

    glEnable(GL_LIGHTING);
}

void SphereWidget::drawRadiusInfo() {
    // В обычном OpenGL без шейдеров отрисовка текста сложна,
    // поэтому информацию о радиусе будем выводить через интерфейс Qt
    // Этот метод оставлен для возможного расширения
}
