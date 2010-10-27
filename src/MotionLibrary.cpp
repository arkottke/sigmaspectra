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
// Copyright 2008 Albert Kottke
////////////////////////////////////////////////////////////////////////////////////


#include "MotionLibrary.h"
#include "Motion.h"
#include "MotionPair.h"

#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>

#include <QSettings>
#include <QtGlobal>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QDirIterator>
#include <QTime>
#include <QtDebug>

#include <limits>


MotionLibrary::MotionLibrary()
{
    // Initialize the variables
    m_motionCount = 0;
    m_disabledCount = 0;
    m_motionsNeedProcessing = true;
    m_worstLoc = -1;

    QSettings settings;
    // Default period vector
    m_damping = settings.value("library/damping", 5.0).toDouble();

    m_periodInterp = settings.value("library/periodInterp", true).toBool();
    m_periodCount = settings.value("library/periodCount", 100).toInt();
    m_periodMin = settings.value("library/periodMin", 0.01).toDouble();
    m_periodMax = settings.value("library/periodMax", 5).toDouble();
    m_periodSpacing = (PeriodSpacing)settings.value("library/periodSpacing", 1).toInt();

    m_oneMotionPerStation = settings.value("library/oneMotionPerStation", true).toBool();
    m_combineComponents = settings.value("library/combineComponents", false).toBool();

    m_seedSize = settings.value("library/seedSize", 2).toInt();
    m_suiteSize = settings.value("library/suiteSize", 7).toInt();
    m_suiteCount = settings.value("library/suiteCount", 10).toInt();
    m_minRequestedCount = settings.value("library/minRequestedCount", 0).toInt();
    
    setMotionPath(settings.value("library/motionPath", "").toString());
}

MotionLibrary::~MotionLibrary()
{
}

QVector<double> & MotionLibrary::inputPeriod()
{
    return m_inputPeriod;
}

QVector<double> & MotionLibrary::inputSa()
{
    return m_inputSa;
}

QVector<double> & MotionLibrary::inputLnStd()
{
    return m_inputLnStd;
}

const QVector<double> & MotionLibrary::period() const
{
    return m_period;
}

const QVector<double> & MotionLibrary::targetSa() const
{
    return m_targetSa;
}

const QVector<double> & MotionLibrary::targetLnStd() const
{
    return m_targetLnStd;
}

const QVector<double> & MotionLibrary::targetSaPlusStd() const
{
    return m_targetSaPlusStd;
}

const QVector<double> & MotionLibrary::targetSaMinusStd() const
{
    return m_targetSaMinusStd;
}

double MotionLibrary::damping() const
{
    return m_damping;
}

void MotionLibrary::setDamping(double damping)
{
    m_damping = damping;
    m_motionsNeedProcessing = true;
}

int MotionLibrary::periodCount() const
{
    return m_periodCount;
}

void MotionLibrary::setPeriodInterp(bool b)
{
    m_motionsNeedProcessing = true;
    m_periodInterp = b;
}

bool MotionLibrary::periodInterp() const
{
    return m_periodInterp;
}

void MotionLibrary::setPeriodCount(int size)
{
    m_motionsNeedProcessing = true;
    m_periodCount = size;
}

double MotionLibrary::periodMin() const
{
    return m_periodMin;
}
void MotionLibrary::setPeriodMin(double min)
{
    m_motionsNeedProcessing = true;
    m_periodMin = min;
}

double MotionLibrary::periodMax() const
{
    return m_periodMax;
}
void MotionLibrary::setPeriodMax(double max)
{
    m_motionsNeedProcessing = true;
    m_periodMax = max;
}

PeriodSpacing MotionLibrary::periodSpacing() const
{
    return m_periodSpacing;
}
void MotionLibrary::setPeriodSpacing(int spacing)
{
    m_motionsNeedProcessing = true;
    m_periodSpacing = (PeriodSpacing)spacing;
}

int MotionLibrary::seedSize() const
{
    return m_seedSize;
}

void MotionLibrary::setSeedSize(int size)
{
    m_seedSize = size;
    // Update the number of motions
    m_trialCount = countTrials();
    emit trialCountChanged(m_trialCount);
}

int MotionLibrary::suiteSize() const
{
    return m_suiteSize;
}

void MotionLibrary::setSuiteSize(int size)
{
    m_suiteSize = size;
    // Update the number of motions
    m_trialCount = countTrials();
    emit trialCountChanged(m_trialCount);
}

int MotionLibrary::suiteCount() const
{
    return m_suiteCount;
}

int MotionLibrary::minRequestedCount() const
{
    return m_minRequestedCount;
}

int MotionLibrary::disabledCount() const
{
    return m_disabledCount;
}

void MotionLibrary::cancel()
{
    m_okToContinue = false;
}

void MotionLibrary::setSuiteCount(int count)
{
    m_suiteCount = count;
}

void MotionLibrary::setMinRequestedCount(int count)
{
    m_minRequestedCount = count;
}

void MotionLibrary::setDisabledCount(int count)
{
    m_disabledCount = count;

    emit motionCountChanged(m_motionCount - m_disabledCount);
    // Update the number of motions
    m_trialCount = countTrials();
    emit trialCountChanged(m_trialCount);
}

bool MotionLibrary::oneMotionPerStation() const
{
    return m_oneMotionPerStation;
}

void MotionLibrary::setOneMotionPerStation(bool b)
{
    m_oneMotionPerStation = b;
}

bool MotionLibrary::combineComponents() const
{
    return m_combineComponents;
}

void MotionLibrary::setCombineComponents(bool b)
{
    m_motionsNeedProcessing = true;
    m_combineComponents = b;

    // Update the number of motions
    m_trialCount = countTrials();
    emit trialCountChanged(m_trialCount);
}

QString MotionLibrary::motionPath() const
{
    return m_motionPath;
}

void MotionLibrary::setMotionPath(const QString & path)
{
    if (!QFile::exists(path))
        return;

    m_motionsNeedProcessing = true;
    m_motionPath = path;

    // Update the number of motions
    m_motionCount = countPath(m_motionPath);
    emit motionCountChanged(m_motionCount - m_disabledCount);

    // Update the number of motions
    m_trialCount = countTrials();
    emit trialCountChanged(m_trialCount);
}

int MotionLibrary::motionCount() const
{
    return m_motionCount;
}

double MotionLibrary::trialCount() const
{
    return m_trialCount;
}

int MotionLibrary::groupSize() const
{
    if (m_combineComponents) {
        return 2;
    } else {
        return 1;
    }
}

QList<AbstractMotion*> & MotionLibrary::motions()
{
    return m_motions;
}

QList<MotionSuite*> & MotionLibrary::suites()
{
    return m_suites;
}

int MotionLibrary::rowCount( const QModelIndex & /*index*/ ) const
{
    return m_suites.size();
}

int MotionLibrary::columnCount( const QModelIndex & /*index*/ ) const
{
    return 4;
}

QVariant MotionLibrary::data( const QModelIndex & index, int role ) const
{
    if (index.parent()!=QModelIndex()) {
        return QVariant();
    }

    if ( role==Qt::BackgroundRole && index.column() == 0 ) {
        if ( data(index, Qt::CheckStateRole).toInt() == Qt::Unchecked ) {
            // Tango Icon Theme -- Scarlet Red
            return QVariant(QBrush(QColor("#EF2929")));
        } else {
            // Tango Icon Theme -- Chameleon
            return QVariant(QBrush(QColor("#8AE234")));
        }
    }

    if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole) {
        switch (index.column()) {
        case 0:
            // Export?
            return QVariant();
        case 1:
            // Rank
            return m_suites.at(index.row())->rank();
        case 2:
            // Median error
            return QString::number(m_suites.at(index.row())->medianError(), 'f', 4);
        case 3:
            // Standard deviation error
            return QString::number(m_suites.at(index.row())->stdevError(), 'f', 4);
        default:
            return QVariant();
        }
    } else if ( role == Qt::CheckStateRole && index.column() == 0) {
        // This needs to return an int for the checkbox to work properly
        if (m_suites.at(index.row())->enabled()) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
        }
    }

    return QVariant();
}

bool MotionLibrary::setData( const QModelIndex & index, const QVariant & value, int role )
{
    if(index.parent()!=QModelIndex()) {
        return false;
    }

    if(role==Qt::EditRole) {
        switch (index.column()) {
        case 1:
            // Rank
            m_suites.at(index.row())->setRank(value.toInt());
            break;
        default:
            return false;
        }
    } else if ( role == Qt::CheckStateRole && index.column() == 0 ) {
        m_suites[index.row()]->setEnabled(value.toBool());
    } else {
        return false;
    }

    emit dataChanged(index, index);
    return true;
}

void MotionLibrary::sort( int column, Qt::SortOrder order )
{
    QMap<QString, MotionSuite*> map;

    // Fill the map with data
    for (int i = 0; i < m_suites.size(); ++i ) {
        map.insertMulti( data( index( i, column), Qt::DisplayRole).toString(), m_suites.at(i) );
    }

    // Grab the sorted data
    QList<MotionSuite*> sortedSuites = map.values();

    // Reverse the sorting it the order is not Ascending
    if ( order != Qt::AscendingOrder) {
        m_suites.clear();
        for (int i = 0; i < sortedSuites.size(); ++i )
            m_suites.prepend(sortedSuites.at(i));
    } else {
        m_suites = sortedSuites;
    }

    // Reset the table
    reset();
}

QVariant MotionLibrary::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole ) {
        return QVariant();
    }

    switch( orientation ) {
    case Qt::Horizontal:
        switch (section) {
                case 0:
            // Export
            return tr("Export");
                case 1:
            // Rank
            return tr("Rank");
                case 2:
            // Median RMSE
            return tr("Median Error");
                case 3:
            // Stdev RMSE
            return tr("Stdev. Error");
                default:
            return QVariant();
        }
    case Qt::Vertical:
        return QVariant(section+1);
    default:
        return QVariant();
    } 
}

Qt::ItemFlags MotionLibrary::flags ( const QModelIndex &index ) const
{
    if ( index.column() == 0 ) {
        return Qt::ItemIsEditable | Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    } else if (index.column() == 1 ) {
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    } else {
        return QAbstractTableModel::flags(index);
    }
}

bool MotionLibrary::interp(const QVector<double> & x, const QVector<double> & y, const QVector<double> & xi, QVector<double> & yi)
{
    // Period (x) has already been checked to ensure that it is increasing
    // Check if all of the data is bounded
    if (std::fabs(xi.first() - x.first()) > std::numeric_limits<double>::epsilon()
        && xi.first() < x.first()) {
        qCritical() << QString(tr("Minimum interpolated value (%1) is less than specified value (%2)").arg(xi.first()).arg(x.first()));
        return false;
    }

    if (std::fabs(xi.first() - x.first()) > std::numeric_limits<double>::epsilon()
        && xi.first() > x.first()) {
        qCritical() << QString(tr("Maximum interpolated value (%1) is greater than specificed value (%2)")).arg(xi.last()).arg(x.last());
        return false;
    }

    //
    // Interpolate using GSL's interpolation method
    //
    yi.resize(xi.size());
    gsl_interp_accel * acc = gsl_interp_accel_alloc ();
    gsl_spline * spline = gsl_spline_alloc(gsl_interp_cspline, x.size());
    gsl_spline_init (spline, x.data(), y.data(), x.size());

    for (int i = 0; i < xi.size(); ++i) {
        yi[i] = gsl_spline_eval(spline, xi.at(i), acc);
    }

    gsl_spline_free(spline);
    gsl_interp_accel_free(acc);

    return true;
}

bool MotionLibrary::readMotions()
{
    m_okToContinue = true;
    // Check the input, if false then there is an error
    if (!isInputValid())
        return false;

    if (m_periodInterp) {
        m_period.resize(m_periodCount);

        if (m_periodSpacing == Linear)
        {
            double delta = (m_periodMax - m_periodMin) / (m_period.size() - 1);
            for (int i = 0; i < m_period.size(); i++) {
                m_period[i] = m_periodMin + delta * i;
            }
        }
        else if (m_periodSpacing == Log)
        {
            double logMax = log10(m_periodMax);
            double logMin = log10(m_periodMin);
            double delta = (logMax - logMin) / (m_period.size() - 1);

            for (int i = 0; i < m_period.size(); i++) {
                m_period[i] = pow(10, logMin + delta * i);
            }
        }

        if (!interp(m_inputPeriod, m_inputSa, m_period, m_targetSa))
            return false;

        if (!interp(m_inputPeriod, m_inputLnStd, m_period, m_targetLnStd))
            return false;
    } else {
        m_period = m_inputPeriod;
        m_targetSa = m_inputSa;
        m_targetLnStd = m_inputLnStd;
    }

    // Compute the target in log space along with the plus and minus one standard deviation
    m_targetLnSa.resize(m_targetSa.size());
    m_targetSaPlusStd.resize(m_targetSa.size());
    m_targetSaMinusStd.resize(m_targetSa.size());

    for (int i = 0; i < m_targetSa.size(); ++i) {
        m_targetLnSa[i] = log(m_targetSa.at(i));
        m_targetSaPlusStd[i] = exp(m_targetLnSa.at(i) + m_targetLnStd.at(i));
        m_targetSaMinusStd[i] = exp(m_targetLnSa.at(i) - m_targetLnStd.at(i));
    }

    if (m_motionsNeedProcessing) {
        emit logText("Processing motion files");

        // Delete previously loaded motions
        while (m_motions.size()) {
            delete m_motions.takeFirst();
        }

        Motion::setPeriod(m_period);
        Motion::setDamping(m_damping/100.);

        QList<Motion*> motions;

        // Read the motion files
        QStringList nameFilters;
        nameFilters << "*.AT2" << "*.at2";

        QDirIterator it(m_motionPath, nameFilters,
                        QDir::AllDirs | QDir::Files | QDir::Readable, QDirIterator::Subdirectories);

        while (it.hasNext()) {
            if (it.next().endsWith(".AT2")) {
                // Update the log
                emit logText("Reading: " + QDir::toNativeSeparators(it.filePath()));
                QApplication::processEvents();
                motions << new Motion(it.filePath());
            }

            if (!m_okToContinue) {
                return false;
            }
        }

        if (m_combineComponents) {
            // Combine motions from the same event and station
            while (motions.size()) {
                Motion * motionA = motions.takeFirst();
                Motion * motionB = 0;

                for (int i = 0; i < motions.size(); ++i) {
                    if (MotionPair::isAPair(motionA, motions.at(i))) {
                        motionB = motions.at(i);
                        break;
                    }
                }

                if (motionB) {
                    m_motions << new MotionPair( motionA, motionB );
                    motions.removeOne(motionB);
                } else {
                    // Remove the motion
                    emit logText("[!] Removing: " + motionA->name() + " no pair found.");
                    delete motionA;
                }

                if (!m_okToContinue) {
                    return false;
                }
            }
        } else {
            while (motions.size()) {
                m_motions << motions.takeFirst();
            }
        }
        // Store that the motion files have been processed
        m_motionsNeedProcessing = false;
    } else {
        emit logText("Using previously processed motion files");

        // The previously plotting may have changed the scale factors of the
        // motions groups, so we need to reset the scale factors back to 1.0.
        for (int i = 0; i < m_motions.size(); ++i) {
            m_motions[i]->scaleBy(1.0);
        }
    }

    return true;
}

int MotionLibrary::countPath(const QString & path)
{	
    int count = 0;

    // Read the motion files
    QStringList nameFilters;
    nameFilters << "*.AT2" << "*.at2";

    QDirIterator it(path, nameFilters, QDir::AllDirs | QDir::Files | QDir::Readable, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        if (it.next().endsWith(".AT2"))
            ++count;
    }

    return count;
}

double MotionLibrary::factorial(const int n) 
{
    return exp(n * log(n) - n 
               + (log(n * (1 + 4 * n * (1 + 2 *n)))) / 6 + log(M_PI)/2);
}

double MotionLibrary::countTrials()
{
    if (m_seedSize > m_suiteSize || m_suiteSize > m_motionCount)
    {
        return 0;
    } else {
        int motionCount = (m_motionCount - m_disabledCount) /
                          (m_combineComponents ?  2 : 1);

        // Count the number of seed motions
        m_seedCount =factorial(motionCount) / (factorial(m_seedSize) * factorial(motionCount-m_seedSize));

        // Count the number of iterative trials
        unsigned long iterCmb = 0;
        for (int i = 0; i < m_suiteSize; ++i)
            iterCmb += motionCount - i;

        if (iterCmb == 0)
            iterCmb = 1;

        // Return the product of the iterative portion and the seed portion
        return m_seedCount * iterCmb;
    }
}


bool lessThan(const MotionSuite* lhs, const MotionSuite* rhs)
{
    return lhs->medianError() < rhs->medianError();
}

void MotionLibrary::save()
{
    QSettings settings;
    settings.setValue("library/periodInterp", m_periodInterp);
    settings.setValue("library/periodCount", m_periodCount);
    settings.setValue("library/periodMin", m_periodMin);
    settings.setValue("library/periodMax", m_periodMax);
    settings.setValue("library/periodSpacing", m_periodSpacing);

    settings.setValue("library/oneMotionPerStation", m_oneMotionPerStation);
    settings.setValue("library/combineComponents", m_combineComponents);

    settings.setValue("library/motionPath", m_motionPath);
    settings.setValue("library/seedSize", m_seedSize);
    settings.setValue("library/suiteSize", m_suiteSize);
    settings.setValue("library/suiteCount", m_suiteCount);
    settings.setValue("library/minRequestedCount", m_minRequestedCount);
}

bool MotionLibrary::compute()
{
    // Read each of the motions
    if (!readMotions()) {
        return false;
    }

    // Select the suites
    emit logText("Selecting suites");
    if (!selectSuites()) {
        return false;
    }

    // Sort the suites from smallest median mse to largest
    qSort(m_suites.begin(), m_suites.end(), lessThan);

    // Scale the selected suites
    if (!m_okToContinue) {
        return false;
    }
    emit logText("Scaling suites");

    for (int i = 0; i < m_suites.size(); i++)  {
        // Scale the suite
        m_suites.at(i)->computeScalars();
        // Update the log with the information
        emit logText(QString("[%1/%2] %3").arg(i+1).arg(m_suites.size()).arg(m_suites.at(i)->errorText()));
    }
    return true;
}

bool MotionLibrary::isInputValid()
{
    if (!m_inputPeriod.size()) {
        qCritical("No target specified");
        return false;
    }

    // Period of the target spectrum must be increasing
    for (int i = 0; i < m_inputPeriod.size()-1; ++i) {
        if (m_inputPeriod.at(i) > m_inputPeriod.at(i+1)) 
        {
            qCritical("The period of the input target spectrum must be an increasing. (T_1 < T_2)");
            return false;
        }
    }

    // Standard deviation must be greater than zero
    for (int i = 0; i < m_inputLnStd.size()-1; ++i) {
        if (m_inputLnStd.at(i) < 0)
        {
            qCritical("The standard deviation of the input target spectrum must be greater than zero.");
            return false;
        }
    }

    // Check the interpolation conditions

    if (m_periodInterp)
    {
        if(m_periodCount < 50) {
            qCritical("Period size for interpolation must be greater than 50.");
            return false;
        }

        if (m_periodMin > m_periodMax) {
            qCritical("Minimum period must be less than the maximum period.");
            return false;
        }

        if (m_periodSpacing == Log && m_periodMin <= 0) {
            qCritical("The minimum interpolated period must be larger than zero with log spacing.");
            return false;
        }

        if (m_periodSpacing == Log && m_inputPeriod.first() <= 0) {
            qCritical("The minimum period of the target spectrum must be larger than zero with log spacing.");
            return false;
        }

        if (m_inputPeriod.first() > m_periodMin) {
            qCritical("Minimum interpolated period is less than minimum period of the target spectrum.");
            return false;
        }

        if (m_inputPeriod.last() < m_periodMax) {
            qCritical("Maximum interpolated period is greater than maximum period of the target spectrum.");
            return false;
        }
    }

    if(m_seedSize < 1) {
        qCritical("Seed size must be at least 1");
        return false;
    }    

    if(m_suiteCount < 1) {
        qCritical("The number of suites to save must be at least 1");
        return false;
    }    

    if(m_suiteSize < 1) {
        qCritical("The numbef of motions in the suite must be at least 1");
        return false;
    }    

    return true;
}

bool MotionLibrary::selectSuites()
{
    // Delete previously selected suites
    while (!m_suites.isEmpty()){
        delete m_suites.takeFirst();
    }

    // Initialize the seed to have values from 0 to n-1
    m_seed.resize(m_seedSize);
    for (int i = 0; i < m_seed.size(); i++){
        m_seed[i] = i;
    }

    // Create a list of required motions
    QList<AbstractMotion*> requiredMotions;

    foreach (AbstractMotion * am, m_motions) {
        if (am->flag() == AbstractMotion::Required)
            requiredMotions << am;
    }

    // Keep track of the percent
    int nextPercent = 1;
    int percent = 0;
    emit percentChanged(percent);
    unsigned long count = 0;
    // Keep track of time to estimate estimated time of completion
    QTime timer;
    QTime now;
    timer.start();

    do {
        // Check if any of the motions in the seed are disabled
        bool badSeed = false;
        for (int i = 0; i < m_seed.size(); i++) {
            if (m_motions.at(m_seed.at(i))->flag() == AbstractMotion::Disabled) {
                badSeed = true;
                break;
            }
        }
        if (badSeed) continue;

        // Create a MotionSuite and add the seed motions
        MotionSuite * ms = new MotionSuite(m_period, m_targetLnSa, m_targetLnStd);
        for (int i = 0; i < m_seed.size(); i++) {
            ms->addMotion(m_motions.at(m_seed.at(i)));
        }

        // Add the one motion that lowers the error the most until the
        // appropriate suite size has been achieved
        bool motionWasAdded = true;
        while(motionWasAdded && ms->motions().size() < m_suiteSize) {
            if (!m_okToContinue) {
                return false;
            }

            // Assume no motion is added
            motionWasAdded = false;

            // Initialized the error
            double minError = 100;
            int minIdx = -1;
            for (int i = 0; i < m_motions.size(); i++)  {
                // Skip if the motion is not valid -- not previously added
                if (!ms->isMotionValid(m_oneMotionPerStation, m_motions.at(i))) {
                    continue;
                }

                // Compute the error with the new motion
                double error = ms->checkMotion(m_motions.at(i));

                // If the error is the smallest value, save the error and the motion index	
                if (error < minError) {
                    minError = error;
                    minIdx = i;
                }
            }

            if (minIdx != -1) {
                // Add the motion that results in the lowest error to the suite
                ms->addMotion(m_motions.at(minIdx));
                motionWasAdded = true;
                //qDebug("Added motion: %i",minIdx);
            } else {
                motionWasAdded = false;
            }
        }
        // Add the suite to the saved suites. If the seed size is the same as
        // the suite size check the suite before adding it
        if (ms->isValid(m_suiteSize, m_minRequestedCount, requiredMotions, m_oneMotionPerStation)) {
            addSuite(ms);
        } else {
            delete ms;
        }

        // Print the status
        count++;
        percent = int(100 * count / m_seedCount);
        if (percent >= nextPercent) {
            // Emit a new percent complete is avaiable
            emit percentChanged(percent);
            // Estimate the time of completion
            now = QTime::currentTime();
            // Emit that a new time is available
            emit timeChanged(now.addMSecs(timer.elapsed() * int(ceil((100-percent)/percent))).toString(Qt::LocalDate));
            // Have the application process the events
            QApplication::processEvents();

            nextPercent = percent + 1;
        }

        if (!m_okToContinue) { 
            // Stop if the user requests it.
            return false;
        }
    } while(nextSeed());
    emit percentChanged(100);
    return true;
}

void MotionLibrary::addSuite(MotionSuite * suite)
{
    if (suite->motions().size() != m_suiteSize) {
        qDebug("Suite not saved -- not enough motions.  Only %i of %i", suite->motions().size(), m_suiteSize);
        delete suite;
        return;
    }

    // Compare the new suite to the previously saved suites.  If the new suite
    // is exactly the same as a previously saved suite, then delete the new
    // suite.
    for (int i = 0; i < m_suites.size(); ++i) {
        int repeats = 0;
        for (int j = 0; j < m_suiteSize; ++j) {
            for (int k = 0; k < m_suiteSize; ++k) {
                if (m_suites.at(i)->motions().at(j) == suite->motions().at(k)) {
                    ++repeats;
                }
            }
            // If all motions are repeated with another suite, delete the current suite.
            if (repeats == m_suiteSize) {
                delete suite;
                return;
            }
        }
    }

    //
    // If the requested number of suites has not been met add the suite
    //
    if (m_suites.size() < m_suiteCount) {
        m_suites.push_back(suite);
    } else if (suite->medianError() < m_worstError) {
        // Remove the worst set
        delete m_suites.at(m_worstLoc);
        // Replace the suite with the worst error with the new suite
        m_suites[m_worstLoc] = suite;
    } else {
        delete suite;
        return;
    }

    // Update the location of the worst error
    m_worstError = m_suites.first()->medianError();
    m_worstLoc = 0;
    for (int i = 1; i < m_suites.size(); i++) {
        if (m_suites.at(i)->medianError() > m_worstError) {
            m_worstError = m_suites.at(i)->medianError();
            m_worstLoc = i;
        }
    }
}

bool MotionLibrary::nextSeed()
{
    if (m_seed[0] == (m_motions.size() - m_seed.size()))
        return false;

    for (int i = m_seed.size() - 1; i >= 0; i--)
    {
        if (m_seed.at(i) < (m_motions.size() - m_seed.size() + i))
        {
            // The value at position i can be increased by one and the
            // remaining values reset
            m_seed[i]++;
            for (int j = i+1; j < m_seed.size(); j++)
                m_seed[j] = m_seed.at(j-1) + 1;
            return true;
        }
    }
    // Shouldn't get here!
    return false;
}

