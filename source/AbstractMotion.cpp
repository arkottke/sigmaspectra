////////////////////////////////////////////////////////////////////////////////////
// This file is part of SigmaSpectra.
//
// SigmaSpectra is free software: you can redistribute it and/or modify it under
// the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// SigmaSpectra is distributed in the hope that it will be useful, but WITHOUT
// ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A
// PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// SigmaSpectra.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2008-2017 Albert Kottke
////////////////////////////////////////////////////////////////////////////////////

#include "AbstractMotion.h"

#include <QObject>

double AbstractMotion::m_damping = 0.;
QVector<double> AbstractMotion::m_period = QVector<double>();

AbstractMotion::AbstractMotion() {
    m_avgLnSa = -1;
    m_prevScale = 1.0;
    m_flag = Unmarked;
}

AbstractMotion::~AbstractMotion() {}

const QString &AbstractMotion::station() const { return m_station; }

const QString &AbstractMotion::event() const { return m_event; }

double AbstractMotion::damping() { return m_damping; }

void AbstractMotion::setDamping(const double damping) { m_damping = damping; }

AbstractMotion::Flag AbstractMotion::flag() const { return m_flag; }

void AbstractMotion::setFlag(Flag flag) { m_flag = flag; }

QList<QString> AbstractMotion::flags() {
    return QList<QString>() << QObject::tr("Required") << QObject::tr("Requested")
                            << QObject::tr("Unmarked") << QObject::tr("Disabled");
}

const QVector<double> &AbstractMotion::period() { return m_period; }

void AbstractMotion::setPeriod(QVector<double> &period) { m_period = period; }

const QVector<double> &AbstractMotion::sa() const { return m_sa; }

const QVector<double> &AbstractMotion::lnSa() const { return m_lnSa; }

double AbstractMotion::avgLnSa() const { return m_avgLnSa; }

void AbstractMotion::scaleBy(const double factor) {
    // Scale factor relative to the previous
    const double relScale = factor / m_prevScale;
    m_prevScale = factor;

    const double lnRelScale = log(relScale);

    for (int i = 0; i < m_sa.size(); ++i) {
        m_sa[i] *= relScale;
        m_lnSa[i] += lnRelScale;
    }

    m_avgLnSa += lnRelScale;
}
