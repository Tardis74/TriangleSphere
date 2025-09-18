#include "coordtransform.h"
#include <cmath>
#include <complex>
#include <limits>
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

        // Проверяем, не являются ли координаты слишком большими
        for (int i = 0; i < 3; ++i) {
            if (std::abs(points[i].x()) > 10000 || std::abs(points[i].y()) > 10000) {
                qWarning() << "Point coordinates too large in transformToSphere:" << points[i];
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

        // Calculate reduced masses with safety checks
        double denominator1 = m1 + m2;
        if (denominator1 <= 0) {
            qWarning() << "Invalid denominator1:" << denominator1;
            return QVector3D(0, 0, 0);
        }
        double mu1 = (m1 * m2) / denominator1;

        double denominator2 = m1 + m2 + m3;
        if (denominator2 <= 0) {
            qWarning() << "Invalid denominator2:" << denominator2;
            return QVector3D(0, 0, 0);
        }
        double mu2 = (m3 * (m1 + m2)) / denominator2;

        // Check for valid reduced masses
        if (std::isnan(mu1) || std::isinf(mu1) || std::isnan(mu2) || std::isinf(mu2)) {
            qWarning() << "Invalid reduced masses:" << mu1 << mu2;
            return QVector3D(0, 0, 0);
        }

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

        // Normalize to get point on sphere
        double norm = std::sqrt(xi1*xi1 + xi2*xi2 + xi3*xi3);

        // Check for invalid norm
        if (norm <= 0 || std::isnan(norm) || std::isinf(norm)) {
            qWarning() << "Invalid norm:" << norm;
            return QVector3D(0, 0, 0);
        }

        return QVector3D(xi1/norm, xi2/norm, xi3/norm);
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

        // Calculate reduced masses with safety checks
        double denominator1 = m1 + m2;
        if (denominator1 <= 0) {
            qWarning() << "Invalid denominator1:" << denominator1;
            return QVector<QPointF>();
        }
        double mu1 = (m1 * m2) / denominator1;

        double denominator2 = m1 + m2 + m3;
        if (denominator2 <= 0) {
            qWarning() << "Invalid denominator2:" << denominator2;
            return QVector<QPointF>();
        }
        double mu2 = (m3 * (m1 + m2)) / denominator2;

        // Extract ξ coordinates
        double xi1 = spherePoint.x();
        double xi2 = spherePoint.y();
        double xi3 = spherePoint.z();

        // Calculate angle λ
        double lambda;
        if (xi2 == 0 && xi3 == 0) {
            lambda = 0; // Avoid division by zero
        } else {
            lambda = std::atan2(xi3, xi2);
        }
        double tgLambda = std::tan(lambda);

        // Calculate magnitudes of Q1 and Q2 using the formulas from the paper
        double normSquared = xi1*xi1 + xi2*xi2 + xi3*xi3;
        if (normSquared < 0) {
            qWarning() << "Negative normSquared:" << normSquared;
            return QVector<QPointF>();
        }
        double sqrtNorm = std::sqrt(normSquared);

        // Calculate Q1 and Q2 magnitudes according to the formulas
        double Q1_magnitude = std::sqrt((sqrtNorm + xi1) / (2.0 * mu1 * (1.0 + tgLambda*tgLambda)));
        double Q2_magnitude = std::sqrt((xi2*xi2 + xi3*xi3) / (2.0 * mu2 * (sqrtNorm + xi1) * (1.0 + tgLambda*tgLambda)));

        if (Q1_magnitude <= 0 || Q2_magnitude <= 0 || std::isnan(Q1_magnitude) || std::isnan(Q2_magnitude)) {
            qWarning() << "Invalid Q1 or Q2 magnitude:" << Q1_magnitude << Q2_magnitude;
            return QVector<QPointF>();
        }

        // Create Q1 and Q2 vectors
        QPointF Q1(Q1_magnitude, 0);
        QPointF Q2(Q2_magnitude * std::cos(lambda), -Q2_magnitude * std::sin(lambda)); // Исправлено: минус перед sin(lambda)

        // Calculate original points according to the formulas
        QPointF r1 = - (m2/denominator1) * Q1 - (m3/denominator2) * Q2;
        QPointF r2 = (m1/denominator1) * Q1 - (m3/denominator2) * Q2;
        QPointF r3 = ((m1 + m2)/denominator2) * Q2;

        // Calculate centroid and scale
        QPointF centroid((r1.x() + r2.x() + r3.x()) / 3, (r1.y() + r2.y() + r3.y()) / 3);

        // Move to origin
        r1 -= centroid;
        r2 -= centroid;
        r3 -= centroid;

        // Scale to desired size
        double currentScale = qMax(qMax(QLineF(r1, r2).length(), QLineF(r2, r3).length()), QLineF(r3, r1).length());
        if (currentScale <= 0 || std::isnan(currentScale) || std::isinf(currentScale)) {
            qWarning() << "Invalid currentScale:" << currentScale;
            return QVector<QPointF>();
        }

        double scaleFactor = scale / currentScale;
        r1 *= scaleFactor;
        r2 *= scaleFactor;
        r3 *= scaleFactor;

        // Move to center
        QPointF center(scale, scale);
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
