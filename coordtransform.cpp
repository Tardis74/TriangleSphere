#include "coordtransform.h"
#include <cmath>
#include <complex>
#include <QDebug>
#include <QtMath>
#include <QLineF>

QVector3D CoordTransform::transformToSphere(const QList<QPointF>& points, const QList<double>& masses) {
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

        // Normalize to get point on sphere
        double norm = std::sqrt(x*x + y*y + z*z);
        if (norm <= 0 || std::isnan(norm) || std::isinf(norm)) {
            qWarning() << "Invalid norm:" << norm;
            return QVector3D(0, 0, 0);
        }

        return QVector3D(x/norm, y/norm, z/norm);
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

        // Scale to desired size
        double currentScale = qMax(qMax(QLineF(r1, r2).length(), QLineF(r2, r3).length()), QLineF(r3, r1).length());
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
