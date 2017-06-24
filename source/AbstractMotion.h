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

#ifndef ABSTRACT_MOTION_H_
#define ABSTRACT_MOTION_H_

#include <QList>
#include <QString>
#include <QVector>
#include <complex>

class AbstractMotion {
public:
    enum Flag {
        Required, //<! Motion required to be in a suite
        Requested, //<! Motion requested to be in a suite
        Unmarked, //<! Default state
        Disabled, //<! Motion should not be used
    };

    AbstractMotion();

    virtual ~AbstractMotion() = 0;

    virtual QString name() const = 0;

    virtual int componentCount() const = 0;

    const QString &station() const;

    const QString &event() const;

    static double damping();

    static void setDamping(const double damping);

    static const QVector<double> &period();

    static void setPeriod(QVector<double> &period);

    Flag flag() const;

    void setFlag(Flag flag);

    static QList<QString> flags();

    const QVector<double> &sa() const;

    const QVector<double> &lnSa() const;

    //! The average logarithm of the response spectrum
    double avgLnSa() const;

    //! Scale the properties of the motion by a factor
    virtual void scaleBy(const double factor);

protected:
    //! Event -- identified by the folder
    QString m_event;

    //! Station -- identified by the filename
    QString m_station;

    //! Response spectrum
    //@{
    //! Damping of the response spectrum
    static double m_damping;

    //! Period of the response spectrum
    static QVector<double> m_period;

    //! Average spectral acceleration of all motions in group
    QVector<double> m_sa;

    //! Natural log of average spectral acceleration
    QVector<double> m_lnSa;
    //@}

    //! Average response over all periods
    double m_avgLnSa;

    //! Previous scale factor applied to the set
    double m_prevScale;

    //! Flag describing the preference of the motion
    Flag m_flag;
};

#endif
