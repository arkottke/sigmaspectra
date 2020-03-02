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

#ifndef MOTION_GROUP_H_
#define MOTION_GROUP_H_

#include "Motion.h"

#include <QList>
#include <QString>
#include <QVector>

/*! MotionGroup allows motions to be combined for uniform scaling allowing
 * motions recored at a given recording station to be given the same scale
 * factor.
 */

class MotionGroup {
public:
    MotionGroup(const QVector<double> &period = QVector<double>(), Motion *motion = 0);

    //        bool operator==( const MotionGroup & other ) const;

    const QString name() const;

    const QVector<double> &period() const;

    const QVector<double> &sa() const;

    const QVector<double> &lnSa() const;

    double avgLnSa() const;

    //! Scale the properties of the motion by a factor
    void scaleBy(double factor);

    void addMotion(Motion *motion);

    QList<Motion *> &motions();

    const QList<Motion *> &motions() const;

protected:
    QList<Motion *> m_motions;

    //! Event
    QString m_event;

    //! Station
    QString m_station;

    //! Period of the response spectrum
    const QVector<double> &m_period;

    //! Average spectral acceleration of all motions in group
    QVector<double> m_sa;

    //! Natural log of average spectral acceleration
    QVector<double> m_lnSa;

    //! Average response over all periods
    double m_avgLnSa;

    //! Previous scale factor applied to the set
    double m_prevScale;
};

#endif
