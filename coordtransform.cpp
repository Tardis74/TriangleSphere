#include "coordtransform.h"
#include <complex>
#include <cmath>
#include <QDebug>
#include <QLineF> // Добавляем недостающий заголовок

QVector3D CoordTransform::transformToSphere(const QList<QPointF>& points, const QList<double>& masses) {
    try {
        QVector3D rawCoords = getRawSphereCoordinates(points, masses);
        if (rawCoords.isNull()) return QVector3D(0, 0, 0);

        // Нормализуем к единичной сфере
        double norm = rawCoords.length();
        if (norm <= 0 || std::isnan(norm) || std::isinf(norm)) {
            return QVector3D(0, 0, 0);
        }

        return rawCoords / norm;
    }
    catch (const std::exception& e) {
        qWarning() << "Exception in transformToSphere:" << e.what();
        return QVector3D(0, 0, 0);
    }
    catch (...) {
        qWarning() << "Unknown exception in transformToSphere";
        return QVector3D(0, 0, 0);
    }
}

QVector3D CoordTransform::getRawSphereCoordinates(const QList<QPointF>& points, const QList<double>& masses) {
    try {
        if (points.size() != 3 || masses.size() != 3) {
            qWarning() << "Invalid points or masses size";
            return QVector3D(0, 0, 0);
        }

        // Check for invalid masses
        for (int i = 0; i < 3; ++i) {
            if (std::isnan(masses[i]) || std::isinf(masses[i]) || masses[i] <= 0) {
                qWarning() << "Invalid mass at index" << i << ":" << masses[i];
                return QVector3D(0, 0, 0);
            }
        }

        // Extract points and masses
        QPointF r1 = points[0];
        QPointF r2 = points[1];
        QPointF r3 = points[2];

        double m1 = masses[0];
        double m2 = masses[1];
        double m3 = masses[2];

        // Calculate reduced masses
        double denominator1 = m1 + m2;
        if (denominator1 <= 0) return QVector3D(0, 0, 0);
        double mu1 = (m1 * m2) / denominator1;

        double denominator2 = m1 + m2 + m3;
        if (denominator2 <= 0) return QVector3D(0, 0, 0);
        double mu2 = (m3 * (m1 + m2)) / denominator2;

        // Calculate vectors Q1 and Q2
        QPointF Q1 = r2 - r1;
        QPointF centerOfMass12 = (m1 * r1 + m2 * r2) / denominator1;
        QPointF Q2 = r3 - centerOfMass12;

        // Convert to complex numbers
        std::complex<double> q1(Q1.x(), Q1.y());
        std::complex<double> q2(Q2.x(), Q2.y());

        // Calculate ξ coordinates
        double xi1 = mu1 * std::norm(q1) - mu2 * std::norm(q2);
        std::complex<double> xi23 = 2.0 * std::sqrt(mu1 * mu2) * q1 * std::conj(q2);
        double xi2 = xi23.real();
        double xi3 = xi23.imag();

        // Меняем порядок координат (xi1, xi2, xi3) -> (xi2, xi3, xi1)
        double x = xi2;
        double y = xi3;
        double z = xi1;

        return QVector3D(x, y, z);
    }
    catch (const std::exception& e) {
        qWarning() << "Exception in getRawSphereCoordinates:" << e.what();
        return QVector3D(0, 0, 0);
    }
    catch (...) {
        qWarning() << "Unknown exception in getRawSphereCoordinates";
        return QVector3D(0, 0, 0);
    }
}

QVector<QPointF> CoordTransform::transformFromSphere(const QVector3D& spherePoint, const QList<double>& masses, double scale) {
    try {
        if (masses.size() != 3) {
            qWarning() << "Invalid masses size";
            return QVector<QPointF>();
        }

        // Check for invalid masses
        for (int i = 0; i < 3; ++i) {
            if (std::isnan(masses[i]) || std::isinf(masses[i]) || masses[i] <= 0) {
                qWarning() << "Invalid mass at index" << i << ":" << masses[i];
                return QVector<QPointF>();
            }
        }

        double m1 = masses[0];
        double m2 = masses[1];
        double m3 = masses[2];

        // Calculate reduced masses
        double denominator1 = m1 + m2;
        if (denominator1 <= 0) return QVector<QPointF>();
        double mu1 = (m1 * m2) / denominator1;

        double denominator2 = m1 + m2 + m3;
        if (denominator2 <= 0) return QVector<QPointF>();
        double mu2 = (m3 * (m1 + m2)) / denominator2;

        // Extract xi coordinates from sphere point
        double xi1 = spherePoint.z();
        double xi2 = spherePoint.x();
        double xi3 = spherePoint.y();

        // Check for valid 1+xi1
        if (1.0 + xi1 <= 0) {
            qWarning() << "Invalid 1+xi1 value:" << 1.0 + xi1;
            return QVector<QPointF>();
        }

        double sqrt_1_xi1 = std::sqrt(1.0 + xi1);
        double sqrt_2_mu1 = std::sqrt(2.0 * mu1);
        double sqrt_2_mu2 = std::sqrt(2.0 * mu2);

        // Calculate Q1 and Q2 vectors with fixed lambda=0
        QPointF Q1(sqrt_1_xi1 / sqrt_2_mu1, 0.0);
        QPointF Q2(xi2 / (sqrt_2_mu2 * sqrt_1_xi1), -xi3 / (sqrt_2_mu2 * sqrt_1_xi1));

        // Calculate original points
        QPointF r1 = - (m2/denominator1) * Q1 - (m3/denominator2) * Q2;
        QPointF r2 = (m1/denominator1) * Q1 - (m3/denominator2) * Q2;
        QPointF r3 = ((m1 + m2)/denominator2) * Q2;

        // Calculate centroid and subtract it
        QPointF centroid((r1.x() + r2.x() + r3.x()) / 3, (r1.y() + r2.y() + r3.y()) / 3);
        r1 -= centroid;
        r2 -= centroid;
        r3 -= centroid;

        // Scale to desired size - исправленная строка с QLineF
        double side1 = std::sqrt(std::pow(r2.x() - r1.x(), 2) + std::pow(r2.y() - r1.y(), 2));
        double side2 = std::sqrt(std::pow(r3.x() - r2.x(), 2) + std::pow(r3.y() - r2.y(), 2));
        double side3 = std::sqrt(std::pow(r1.x() - r3.x(), 2) + std::pow(r1.y() - r3.y(), 2));
        double currentScale = qMax(side1, qMax(side2, side3));
        
        if (currentScale <= 0 || std::isnan(currentScale) || std::isinf(currentScale)) {
            return QVector<QPointF>();
        }

        double scaleFactor = scale / currentScale;
        r1 *= scaleFactor;
        r2 *= scaleFactor;
        r3 *= scaleFactor;

        // Move to center
        QPointF center(scale/2, scale/2);
        r1 += center;
        r2 += center;
        r3 += center;

        return {r1, r2, r3};
    }
    catch (const std::exception& e) {
        qWarning() << "Exception in transformFromSphere:" << e.what();
        return QVector<QPointF>();
    }
    catch (...) {
        qWarning() << "Unknown exception in transformFromSphere";
        return QVector<QPointF>();
    }
}

QVector<ComplexSolution> CoordTransform::transformZetaToZ(const QPointF& zetaPoint)
{
    QVector<ComplexSolution> solutions;
    
    try {
        std::complex<double> zeta(zetaPoint.x(), zetaPoint.y());
        
        // Вычисляем промежуточные величины
        std::complex<double> zeta2 = zeta * zeta;
        std::complex<double> inner_sqrt = std::sqrt(zeta2 - zeta + 1.0);
        
        // 4 комбинации знаков
        std::complex<double> signs[4][2] = {
            {+1.0, -1.0}, // ветвь 0: +, -
            {+1.0, +1.0}, // ветвь 1: +, +
            {-1.0, -1.0}, // ветвь 2: -, -
            {-1.0, +1.0}  // ветвь 3: -, +
        };
        
        QColor colors[4] = {
            QColor(255, 0, 0),    // Красный - ветвь 0
            QColor(0, 255, 0),    // Зеленый - ветвь 1  
            QColor(0, 0, 255),    // Синий - ветвь 2
            QColor(255, 165, 0)   // Оранжевый - ветвь 3
        };
        
        for (int i = 0; i < 4; ++i) {
            std::complex<double> sign1 = signs[i][0];
            std::complex<double> sign2 = signs[i][1];
            
            // Вычисляем выражение под корнем в числителе
            std::complex<double> inner_expr = sign1 * 2.0 * (zeta + 1.0) * inner_sqrt + 2.0 * zeta2 + zeta - 1.0;
            
            // Проверяем, что выражение под корнем не отрицательное (в вещественном смысле)
            if (std::abs(inner_expr) < 1e10) { // Фильтруем слишком большие значения
                std::complex<double> numerator_sqrt = std::sqrt(inner_expr);
                
                // Вычисляем z_i согласно формуле (4.6)
                std::complex<double> z_i = (sign1 * numerator_sqrt / std::sqrt(2.0))
                                         + (sign2 * inner_sqrt / std::sqrt(2.0))
                                         - (zeta / std::sqrt(2.0));
                
                // Фильтруем валидные решения (не NaN и не бесконечность)
                if (!std::isnan(z_i.real()) && !std::isnan(z_i.imag()) &&
                    !std::isinf(z_i.real()) && !std::isinf(z_i.imag())) {
                    
                    // Ограничиваем значения для отображения
                    double real_part = std::max(-5.0, std::min(5.0, z_i.real()));
                    double imag_part = std::max(-5.0, std::min(5.0, z_i.imag()));
                    
                    ComplexSolution solution;
                    solution.point = QPointF(real_part, imag_part);
                    solution.color = colors[i];
                    solution.branch = i;
                    solutions.append(solution);
                }
            }
        }
        
        qDebug() << "Found" << solutions.size() << "solutions for ζ =" << zetaPoint.x() << "+ i" << zetaPoint.y();
        
    } catch (const std::exception& e) {
        qWarning() << "Exception in transformZetaToZ:" << e.what();
    }
    
    return solutions;
}

QPointF CoordTransform::transformZToZeta(const QPointF& zPoint)
{
    try {
        std::complex<double> z(zPoint.x(), zPoint.y());
        
        // Прямое преобразование: ζ = z(√8 + z³)/(1 - √8 z³)
        double sqrt8 = std::sqrt(8.0);
        std::complex<double> z3 = z * z * z;
        
        // Проверяем знаменатель
        std::complex<double> denominator = 1.0 - sqrt8 * z3;
        if (std::abs(denominator) < 1e-10) {
            return QPointF(0, 0); // Избегаем деления на ноль
        }
        
        std::complex<double> zeta = z * (sqrt8 + z3) / denominator;
        
        // Ограничиваем значения
        double real_part = std::max(-2.0, std::min(2.0, zeta.real()));
        double imag_part = std::max(-2.0, std::min(2.0, zeta.imag()));
        
        return QPointF(real_part, imag_part);
        
    } catch (const std::exception& e) {
        qWarning() << "Exception in transformZToZeta:" << e.what();
        return QPointF(0, 0);
    }
}
