////////////////////////////////////////////////////////////////////////////////////
// This file is part of SigmaSpectra.
//
// SigmaSpectra is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// SigmaSpectra is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// SigmaSpectra.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2008-2017 Albert Kottke
////////////////////////////////////////////////////////////////////////////////////

#include "MotionSuite.h"
#include "MotionPair.h"

#include <QtDebug>

#include <gsl/gsl_cdf.h>

MotionSuite::MotionSuite(const QVector<double> &period, const QVector<double> &targetLnSa,
                         const QVector<double> &targetLnStd)
        : m_period(period), m_targetLnSa(targetLnSa), m_targetLnStd(targetLnStd), 
        m_medianError(-1), m_stdevError(-1), m_medianMaxError(-1), m_sigmaInf(-1) {
    m_rank = 0;
    m_enabled = false;
}

MotionSuite::~MotionSuite() {
}

QStringList MotionSuite::outputTypes() {
    QStringList list;
    list << "No Output"
         << "Strata"
         << "CSV"
         << "SHAKE2000";

    return list;
}

int MotionSuite::rank() const {
    return m_rank;
}

void MotionSuite::setRank(int rank) {
    m_rank = rank;
}

bool MotionSuite::enabled() const {
    return m_enabled;
}

void MotionSuite::setEnabled(bool enabled) {
    m_enabled = enabled;
}

double MotionSuite::medianError() const {
    return m_medianError;
}

double MotionSuite::stdevError() const {
    return m_stdevError;
}

const QString MotionSuite::errorText() const {
    return QString("Median RMSE: %1   Max Error: %2%   Std RMSE: %3   Sigma Inf: %4")
            .arg(m_medianError, 6, 'f', 4)
            .arg(m_medianMaxError, 6, 'f', 4)
            .arg(m_stdevError, 6, 'f', 4)
            .arg(m_sigmaInf, 5, 'f', 3);
}

const QVector<double> &MotionSuite::avgSa() const {
    return m_avgSa;
}

const QVector<double> &MotionSuite::lnStd() const {
    return m_lnStd;
}

QVector<double> MotionSuite::fractile(double eps) const {
    QVector<double> values(m_lnAvg.size());
    for (int i = 0; i < m_lnAvg.size(); ++i) {
        values[i] = exp(eps * m_lnStd.at(i) + m_lnAvg.at(i));
    }
    return values;
}

const QList<AbstractMotion *> &MotionSuite::motions() const {
    return m_motions;
}

const QVector<double> &MotionSuite::scalars() const {
    return m_scalars;
}

bool MotionSuite::isValid(const int suiteSize, const int minRequestedCount,
                          const QList<AbstractMotion *> &requiredMotions, const bool oneMotionPerStation) const {
    // Check the suite size
    if (suiteSize != m_motions.size()) {
        return false;
    }

    // Check for the required number of requested motions
    int count = 0;
    for (const AbstractMotion *am : m_motions) {
        if (am->flag() == AbstractMotion::Requested) {
            ++count;
        }
    }

    if (count < minRequestedCount) {
        return false;
    }

    // Check that all of the required motions are present
    for (AbstractMotion *am : requiredMotions) {
        if (m_motions.contains(am) == false) {
            return false;
        }
    }

    // While this has been checked in the incremental process, it still needs
    // to be checked for totally random suites.
    for (int i = 0; i < m_motions.size(); ++i) {
        if (isMotionValid(oneMotionPerStation, m_motions.at(i), i) == false) {
            return false;
        }
    }

    return true;
}

bool MotionSuite::isMotionValid(const bool oneMotionPerStation,
                                const AbstractMotion *motion, const int motionIndex) const {
    if (motion->flag() == AbstractMotion::Disabled) {
        return false;
    }

    for (int i = 0; i < m_motions.size(); ++i) {
        if (motionIndex == i) {
            continue;
        }
        // Check if the motion is repeated by comparing the pointer
        if (motion == m_motions.at(i)) {
            return false;
        }
        // Check if the motion has been used already by checking if the event
        // and station match any other motion
        if (oneMotionPerStation
            && motion->station() == m_motions.at(i)->station()) {
            return false;
        }
    }

    return true;
}

double MotionSuite::checkMotion(const AbstractMotion *motion) const {
    QVector<double> lnAvg(m_lnAvg.size());
    // Compute the median of the response spectra
    int n = m_motions.size() + 1;
    for (int i = 0; i < m_lnAvg.size(); ++i) {
        lnAvg[i] = m_lnAvg.at(i) * (n - 1) / n + motion->lnSa().at(i) / n;
    }

    return computeError(lnAvg, m_targetLnSa);
}

void MotionSuite::addMotion(AbstractMotion *motion) {
    m_motions.push_back(motion);
    // Compute the average value
    if (m_motions.size() > 1) {
        // Compute the average by combining the previous average with the new value
        int n = m_motions.size();
        for (int i = 0; i < m_lnAvg.size(); i++) {
            m_lnAvg[i] = m_lnAvg.at(i) * (n - 1) / n + motion->lnSa().at(i) / n;
        }
    } else {
        m_lnAvg.resize(m_targetLnSa.size());
        // Copy the value over
        for (int i = 0; i < m_lnAvg.size(); i++) {
            m_lnAvg[i] = motion->lnSa().at(i);
        }
    }
    // Recompute the median RMSE
    m_medianError = computeError(m_lnAvg, m_targetLnSa, &m_medianMaxError);
}

bool lessThan(const AbstractMotion *motionA, const AbstractMotion *motionB) {
    return motionA->avgLnSa() < motionB->avgLnSa();
}

void MotionSuite::computeScalars() {
    //Sort the motions from the smallest average spectral response to the largest.
    std::sort(m_motions.begin(), m_motions.end(), lessThan);

    //The fractiles are computed for a standard normal distribution with a zero
    //mean and a standard deviation of 1.  The fractiles are ordered from
    //largest to smallest.

    QVector<double> centroids = calcCentroid();

    // Check if a zero sigma value is specified
    bool zeroSigmaValue = false;
    for (int i = 0; i < m_targetLnStd.size(); ++i) {
        if (m_targetLnStd.at(i) == 0) {
            zeroSigmaValue = true;
            break;
        }
    }

    double minScale = -1;
    if (zeroSigmaValue) {
        minScale = 1.0;
    } else {
        double minError = 100;
        for (double scale = 0.10; scale < 3; scale += 0.01) {
            double error = computeStdError(scale, centroids);
            if (error < minError) {
                minError = error;
                minScale = scale;
            }
        }
    }

    // Update the RMSE of the suite, compute the scalars, and return the mean of square errors
    m_sigmaInf = minScale;
    m_stdevError = computeStdError(minScale, centroids);

    // Sort the motions and the scalars by the name of the motion
    QMap<QString, AbstractMotion *> motionMap;
    QMap<QString, double> scalarMap;

    for (int i = 0; i < m_motions.size(); ++i) {
        motionMap.insert(m_motions.at(i)->name(), m_motions.at(i));
        scalarMap.insert(m_motions.at(i)->name(), m_scalars.at(i));
    }

    m_motions = motionMap.values();
    m_scalars = scalarMap.values().toVector();
}

void MotionSuite::scaleMotions() {
    for (int i = 0; i < m_motions.size(); ++i) {
        m_motions[i]->scaleBy(m_scalars.at(i));
    }
}

void MotionSuite::toText(QTextStream &os, MotionSuite::OutputType type) {
    if (type == SummaryOutput || type == CSVOutput) {
        scaleMotions();
        // Print the error information
        os << QString("Median RMSE,%1\nMedian Max Error (%),%2\nStd RMSE,%3\nSigma Inf,%4\n")
                .arg(m_medianError)
                .arg(m_medianMaxError)
                .arg(m_stdevError)
                .arg(m_sigmaInf);

        // Print a table of the scaled motions
        for (int i = 0; i < columnCount(); ++i) {
            os << headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() << ",";
        }
        os << endl;

        for (int row = 0; row < rowCount(); ++row) {
            for (int col = 0; col < columnCount(); ++col) {
                os << data(index(row, col), Qt::UserRole).toString() << ",";
            }
            os << endl;
        }

        if (type == CSVOutput) {
            // Print out the response spectra headers
            os << "\n\nPeriod (s),Median Sa (g),Sigma_ln";
            for (int j = 0; j < rowCount(); ++j) {
                os << "," << selectMotion(j)->name();
            }
            os << endl;

            // Print the response spectra data
            for (int i = 0; i < m_period.size(); ++i) {
                // Print the period, median, and standard deviation
                os << m_period.at(i) << "," << exp(m_lnAvg.at(i)) << "," << m_lnStd.at(i);
                // Print out the individual motions
                for (int j = 0; j < rowCount(); ++j) {
                    os << "," << selectMotion(j)->sa().at(i);
                }
                os << endl;
            }
        }
    } else if (type == StrataOutput) {
        for (int i = 0; i < rowCount(); ++i) {
            os << selectMotion(i)->fileName() << "," << selectScalar(i) << endl;
        }
    } else if (type == SHAKE2000Output) {
        os << QString("Median RMSE: %1 Max Error: %2% Std RMSE: %3 Sigma Inf: %4")
                .arg(m_medianError, 6, 'f', 4)
                .arg(m_medianMaxError, 6, 'f', 3)
                .arg(m_stdevError, 6, 'f', 4)
                .arg(m_sigmaInf, 4, 'f', 2)
           << endl;

        os << QString("%1%2").arg("Motion", -80).arg("Scale", -6) << endl;

        // Used to convert between a single motion and paired motion
        for (int i = 0; i < rowCount(); ++i) {
            os << QString("%1%2")
                    .arg(selectMotion(i)->name(), -80)
                    .arg(selectScalar(i), -6, 'f', 3)
               << endl;
        }
    }
}

int MotionSuite::rowCount(const QModelIndex & /*index*/) const {
    return m_motions.size() * m_motions.first()->componentCount();
}

int MotionSuite::columnCount(const QModelIndex & /*index*/) const {
    return 8;
}

QVariant MotionSuite::data(const QModelIndex &index, int role) const {
    if (index.parent() != QModelIndex()) {
        return QVariant();
    }

    // Higher precision for user role
    int precision = (role == Qt::DisplayRole) ? 2 : 4;

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        switch (index.column()) {
            case 0:
                // Name
                return selectMotion(index.row())->name();
            case 1:
                // Scale
                return QString::number(selectScalar(index.row()), 'f', precision);
            case 2:
                // PGA
                return QString::number(selectMotion(index.row())->pga(), 'f', precision);
            case 3:
                // PGV
                return QString::number(selectMotion(index.row())->pgv(), 'f', precision);
            case 4:
                // PGD
                return QString::number(selectMotion(index.row())->pgd(), 'f', precision);
            case 5:
                // Dur 5-75
                return QString::number(selectMotion(index.row())->dur5_75(), 'f', precision);
            case 6:
                // Dur 5-95
                return QString::number(selectMotion(index.row())->dur5_95(), 'f', precision);
            case 7:
                // Details
                if (role == Qt::UserRole) {
                    return "\"" + selectMotion(index.row())->details() + "\"";
                } else {
                    return selectMotion(index.row())->details();
                }
            default:
                return QVariant();
        }
    }

    return QVariant();
}

QVariant MotionSuite::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole) {
        return QVariant();
    }

    switch (orientation) {
        case Qt::Horizontal:
            switch (section) {
                case 0:
                    // Name
                    return tr("Name");
                case 1:
                    // Scale
                    return tr("Scale");
                case 2:
                    // PGA
                    return tr("PGA (g)");
                case 3:
                    // PGV
                    return tr("PGV (cm/s)");
                case 4:
                    // PGD
                    return tr("PGD (cm)");
                case 5:
                    // Dur 5-75
                    return tr("Dur. 5-75 (s)");
                case 6:
                    // Dur 5-95
                    return tr("Dur. 5-95 (s)");
                case 7:
                    // Details
                    return tr("Details");
                default:
                    return QVariant();
            }

        case Qt::Vertical:
            return QVariant(section + 1);
        default:
            return QVariant();
    }
}

double MotionSuite::computeError(const QVector<double> &vec, const QVector<double> &ref, double *maxError) {
    // Compute the optimal scale factor
    double sum = 0;
    for (int i = 0; i < vec.size(); ++i) {
        sum += ref.at(i) - vec.at(i);
    }
    double scalar = sum / vec.size();

    /*
         *  Compute the sum of square error (SSE) bewteen the scaled suite and
         *  the target.  The error is not divided by the target value because the
         *  target value goes to zero at times and that would cause problems.
         */
    double sse = 0;
    for (int i = 0; i < vec.size(); ++i) {
        sse += pow(scalar + vec.at(i) - ref.at(i), 2);
    }

    /*
         * Compute the maximum error
         */
    if (maxError != NULL) {
        // Initialize max error
        *maxError = -1;
        for (int i = 0; i < vec.size(); ++i) {
            double error = 100 * fabs(exp(scalar + vec.at(i)) - exp(ref.at(i))) / exp(ref.at(i));
            // Save the error
            if (*maxError < 0 || *maxError < error) {
                *maxError = error;
            }
        }
    }

    // Return the root mean square error
    return sqrt(sse / vec.size());
}

QVector<double> MotionSuite::calcCentroid() {
    QVector<double> centroid(m_motions.size());

    // Number of slices for each section
    const int count = 20;

    // Change in the probability for each slice
    const double dProb = 1.0 / m_motions.size();
    const double minProb = 0.000001;

    for (int i = 0; i < m_motions.size(); ++i) {
        // Compute the bounding probabilities of the slice
        const double probL = (i == 0) ? minProb : i * dProb;
        const double probR = (i == m_motions.size() - 1) ? (1 - minProb) : (i + 1) * dProb;

        // Compute the x values corresponding to the probabilities
        const double xL = gsl_cdf_ugaussian_Pinv(probL);
        const double xR = gsl_cdf_ugaussian_Pinv(probR);

        // Integration step
        const double du = (xR - xL) / (count - 1);
        double moment = 0;
        for (int j = 0; j < count - 1; ++j) {
            double uR = xL + (j + 1) * du;
            double uL = xL + j * du;
            double area = gsl_cdf_ugaussian_P(uR) - gsl_cdf_ugaussian_P(uL);
            moment += area * (uL + uR) / 2;
        }
        centroid[i] = moment / dProb;
    }
    return centroid;
}

const Motion *MotionSuite::selectMotion(int index) const {
    const AbstractMotion *am;

    if (dynamic_cast<Motion *>(m_motions.first())) {
        am = m_motions.at(index);
    } else {
        const MotionPair *mp = static_cast<MotionPair *>(m_motions.at(index / 2));
        am = (index % 2) ? mp->motionB() : mp->motionA();
    }

    return static_cast<const Motion *>(am);
}

double MotionSuite::selectScalar(int index) const {
    return m_scalars.at(index / ((dynamic_cast<Motion *>(m_motions.first())) ? 1 : 2));
}

/*
 * Apply the scalars to the motions, compute the standard deviation and the
 * mean of squared errors of the standard deviation.
 */
double MotionSuite::computeStdError(const double sigmaScalar, const QVector<double> &centroids) {
    // Check that the size of the scalars and motions matches
    if (m_scalars.size() != m_motions.size()) {
        m_scalars.resize(m_motions.size());
    }
    /*
         * Scale each of the motions to the appropriate fractile
         */
    for (int i = 0; i < m_motions.size(); i++) {
        double sum = 0;
        for (int j = 0; j < m_targetLnStd.size(); j++) {
            double fractile = m_targetLnSa.at(j) + sigmaScalar * m_targetLnStd.at(j) * centroids.at(i);
            // Compute the sum of the difference between the fractile and the motion
            sum += fractile - m_motions.at(i)->lnSa().at(j);
        }
        // Compute the scalar value in linear space
        m_scalars[i] = exp(sum / m_targetLnStd.size());
    }
    /*
         * Compute the new average of the suite
         */
    for (int i = 0; i < m_lnAvg.size(); i++) {
        double sum = 0;
        for (int j = 0; j < m_motions.size(); j++) {
            sum += m_motions.at(j)->lnSa().at(i) + log(m_scalars.at(j));
        }
        m_lnAvg[i] = sum / m_motions.size();
    }

    /*
     * Compute the standard deviation of the suite
     */
    if (m_lnStd.size() != m_targetLnStd.size()) {
        m_lnStd.resize(m_targetLnStd.size());
    }
    double sse = 0;
    for (int i = 0; i < m_targetLnStd.size(); i++) {
        // The variance is the sum of (x-avg)^2 * p(x).  The probability for
        // all x's is equal and equal to 1 over the number of values
        double var = 0;
        for (int j = 0; j < m_motions.size(); j++) {
            var += pow(m_motions.at(j)->lnSa().at(i) + log(m_scalars.at(j)) - m_lnAvg.at(i), 2);
        }
        // The standard deviation is the square root of the variance
        m_lnStd[i] = sqrt(var / (m_motions.size() - 1));
        // Compute the error between the computed and target standard deviation
        sse += pow(m_lnStd.at(i) - m_targetLnStd.at(i), 2);
    }
    // Return the root mean square error
    return sqrt(sse / m_targetLnStd.size());
}
