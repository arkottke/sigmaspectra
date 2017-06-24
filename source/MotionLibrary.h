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

#ifndef MOTIONLIBRARY_H_
#define MOTIONLIBRARY_H_

#include "MotionGroup.h"
#include "MotionSuite.h"

#include <QAbstractTableModel>
#include <QLineEdit>
#include <QList>
#include <QProgressBar>
#include <QString>
#include <QVector>

enum PeriodSpacing {
    Linear,
    Log
};

class MotionLibrary : public QAbstractTableModel {
Q_OBJECT

public:
    MotionLibrary();

    ~MotionLibrary();

    QVector<double> &inputPeriod();

    QVector<double> &inputSa();

    QVector<double> &inputLnStd();

    const QVector<double> &period() const;

    const QVector<double> &targetSa() const;

    const QVector<double> &targetLnStd() const;

    QVector<double> targetFracile(double eps) const;

    double damping() const;

    bool periodInterp() const;

    int periodCount() const;

    double periodMin() const;

    double periodMax() const;

    PeriodSpacing periodSpacing() const;

    int suiteSize() const;

    int seedSize() const;

    int suiteCount() const;

    int minRequestedCount() const;

    int disabledCount() const;

    QString motionPath() const;

    bool oneMotionPerStation() const;

    bool combineComponents() const;

    int motionCount() const;

    double trialCount() const;

    int groupSize() const;

    QList<AbstractMotion *> &motions();

    QList<MotionSuite *> &suites();

    int rowCount(const QModelIndex &index = QModelIndex()) const;

    int columnCount(const QModelIndex &index = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    //! Save the properties to the settings
    void save();

    //! Start the calculation
    bool compute();

    //! Read the motions from the files and create the motionGroups
    bool readMotions();

public slots:

    void setDamping(double damping);

    void setMotionPath(const QString &path);

    void setSuiteSize(int size);

    void setSeedSize(int size);

    void setSuiteCount(int count);

    void setMinRequestedCount(int count);

    void setDisabledCount(int count);

    void setPeriodInterp(bool);

    void setPeriodCount(int size);

    void setPeriodMin(double min);

    void setPeriodMax(double max);

    void setPeriodSpacing(int spacing);

    void setOneMotionPerStation(bool b);

    void setCombineComponents(bool b);

    void cancel();

signals:

    void logText(const QString &);

    void percentChanged(int percent);

    void timeChanged(const QString &etc);

    void motionCountChanged(int);

    void trialCountChanged(double);

private:
    /*! Log-log interpolation.
         * \param x x values
         * \param y y values
         * \param xi x values to interpolate values at
         * \param yi interpolated y values
         */
    static bool
    interp(const QVector<double> &x, const QVector<double> &y, const QVector<double> &xi, QVector<double> &yi);

    //! If it is okay to continue the calcuation
    bool m_okToContinue;

    /*! Select the suites
         * \return true if the operation was successful
         */
    bool selectSuites();

    /*! Scale the selected suites to the target standard deviation
         * \return true if the operation was successful
         */
    bool scaleSuites();

    /*! Count the number of motions found in the path
         * \return the number of motions found
         */
    int countPath(const QString &path);

    bool isInputValid();

    bool isSuiteValid(const MotionSuite *temp_ms, const MotionGroup *motionGroup);

    bool isSuiteValid(const MotionSuite *temp_ms);

    double countTrials();

    void addSuite(MotionSuite *suite);

    //! Compute the next seed
    bool nextSeed();

    bool m_motionsNeedProcessing;
    QString m_motionPath;

    //! Motions read from files
    QList<AbstractMotion *> m_motions;

    //! Damping of the oscillator in percent
    double m_damping;

    //! Input target specified by the user
    //@{
    QVector<double> m_inputPeriod;
    QVector<double> m_inputSa;
    QVector<double> m_inputLnStd;
    //@}

    bool m_periodInterp;
    int m_periodCount;
    double m_periodMin;
    double m_periodMax;
    PeriodSpacing m_periodSpacing;

    QVector<double> m_period;
    QVector<double> m_targetSa;
    QVector<double> m_targetSaPlusStd;
    QVector<double> m_targetSaMinusStd;
    QVector<double> m_targetLnSa;
    QVector<double> m_targetLnStd;

    int m_motionCount;
    int m_disabledCount;
    int m_suiteCount;
    int m_suiteSize;
    int m_seedSize;
    double m_trialCount;
    double m_seedCount;

    //! Minimum of marked motions required for the suite
    int m_minRequestedCount;

    QVector<int> m_seed;
    QList<MotionSuite *> m_suites;

    /*! Only permit one component per recording station for each event.
     */
    bool m_oneMotionPerStation;

    /*! Combine components of a recording station for each event.
     * Allow the program to select motions for two dimensional analysis.
     */
    bool m_combineComponents;

    // The worst error (RMSE) value and the location are stored to be quickly replaced with new suites
    int m_worstLoc;
    double m_worstError;

    /*! Compute the factorial of an integer.
     * The factorial of an integer is computed using the Srinivasa Ramanujan approximation.
     *
     * \param n integer
     * \return approximation of the factorial
     */
    static double factorial(const int n);
};

#endif
