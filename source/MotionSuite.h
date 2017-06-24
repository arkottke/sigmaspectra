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

#ifndef MOTION_SUITE_H_
#define MOTION_SUITE_H_

#include "AbstractMotion.h"
#include "Motion.h"

#include <QAbstractTableModel>
#include <QList>
#include <QTextStream>

class MotionSuite : public QAbstractTableModel {
Q_OBJECT

public:
    enum OutputType {
        NoOutput, //!< No output created
        SummaryOutput, //!< Only a summary of the motions
        StrataOutput, //!< Output suitable for input into Strata
        CSVOutput, //!< Comma-separated-values suitable for Excel
        SHAKE2000Output //!< Output suitable for input into SHAKE2000
    };

    MotionSuite(const QVector<double> &period, const QVector<double> &targetLnSa, const QVector<double> &targetLnStd);

    ~MotionSuite();

    static QStringList outputTypes();

    int rank() const;

    void setRank(int rank);

    bool enabled() const;

    void setEnabled(bool enabled);

    double medianError() const;

    double stdevError() const;

    const QString errorText() const;

    const QVector<double> &avgSa() const;

    const QVector<double> &lnStd() const;

    QVector<double> fractile(double eps) const;

    const QList<AbstractMotion *> &motions() const;

    const QVector<double> &scalars() const;

    /*! Check if the suite is valid.
         */
    bool isValid(const int suiteSize, const int minRequestedCount, const QList<AbstractMotion *> &requiredMotions,
                 const bool oneMotionPerStation) const;

    /*! Test if the motion could be added to the suite.
         */
    bool isMotionValid(const bool oneMotionPerStation, const AbstractMotion *motion, const int motionIndex = -1) const;

    /*! Check the fit of the median response spectrum with a new motion
         * \param motion new potential motion
         * \return root mean square error of the median
         */
    double checkMotion(const AbstractMotion *motion) const;

    //! Permantly add a motion to the suite
    void addMotion(AbstractMotion *motion);

    //! Compute the scalar factors to fit the target spectrum
    void computeScalars();

    //! Scale the motions in the suite for plotting
    void scaleMotions();

    void toText(QTextStream &os, OutputType type);

    int rowCount(const QModelIndex &index = QModelIndex()) const;

    int columnCount(const QModelIndex &index = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    /*! Select a motion for a given index.
         */
    const Motion *selectMotion(int index) const;

    /*! Select the proper scale for a given index.
         */
    double selectScalar(int index) const;

private:
    /*! Compute the error between a vector and a reference.
         * Compute the root-mean-square error and the maximum percent error
         * \param vec vector whose error is to be computed
         * \param ref reference vector
         * \param maximumError reference to maximum error found
         * \return root mean sum of square error
         */
    static double computeError(const QVector<double> &vec, const QVector<double> &ref, double *maxError = NULL);

    //! Enabled for output
    bool m_enabled;

    //! Compute the error in standard deviation
    double computeStdError(const double sigmaScalar, const QVector<double> &centroids);

    //! Compute the centroids for the suite
    QVector<double> calcCentroid();

    //! User defined rank of the suite
    int m_rank;

    //! Target spectrum
    //!@{
    //! Period
    const QVector<double> &m_period;
    //! Natual log of the target spectral acceleration
    const QVector<double> &m_targetLnSa;
    //! Standard deviation of the natual log of the target spectral acceleration
    const QVector<double> &m_targetLnStd;
    //!@}

    //! List of the motions
    QList<AbstractMotion *> m_motions;

    //! Scale factors for the motions
    QVector<double> m_scalars;

    //! Median response spectrum of the suite
    QVector<double> m_avgSa;

    //! Natural log of the average spectral acceleration
    QVector<double> m_lnAvg;

    //! Standard deviation of the natural log of the spectral accelerations
    QVector<double> m_lnStd;

    //! Root mean square error of the median
    double m_medianError;

    //! Root mean square error of the standard deviation
    double m_stdevError;

    //! Maximum error in the median
    double m_medianMaxError;

    //! Factor used to adjust the standard deviation
    double m_sigmaInf;
};

#endif
