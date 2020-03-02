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

#include "MotionGroup.h"

#include <QDir>
#include <QtDebug>

MotionGroup::MotionGroup(const QVector<double> &period, Motion *motion)
        : m_period(period) {
    m_prevScale = 1;

    if (motion) {
        addMotion(motion);
    }
}


const QString MotionGroup::name() const {
    QString name = m_event + QDir::separator() + m_station;

    if (m_motions.size() == 1) {
        name += m_motions.first()->component();
    }

    return name;
}

const QVector<double> &MotionGroup::period() const {
    return m_period;
}

const QVector<double> &MotionGroup::sa() const {
    return m_sa;
}

const QVector<double> &MotionGroup::lnSa() const {
    return m_lnSa;
}

double MotionGroup::avgLnSa() const {
    return m_avgLnSa;
}

void MotionGroup::scaleBy(double factor) {
    // Scale factor relative to the previous
    const double relScale = factor / m_prevScale;
    m_prevScale = factor;

    const double logRelScale = log(relScale);

    for (int i = 0; i < m_sa.size(); ++i) {
        m_sa[i] *= relScale;
        m_lnSa[i] += logRelScale;
    }

    m_avgLnSa += logRelScale;

    for (int i = 0; i < m_motions.size(); ++i) {
        m_motions[i]->scaleBy(factor);
    }
}

void MotionGroup::addMotion(Motion *motion) {
    if (m_motions.size() == 0) {
        m_lnSa.resize(motion->lnSa().size());
        m_sa.resize(m_lnSa.size());

        m_event = motion->event();
        m_station = motion->station();
    } else {
        Q_ASSERT(m_event == motion->event());
        Q_ASSERT(m_station == motion->station());
    }

    m_motions << motion;

    const int n = m_motions.size();
    double sum = 0;

    for (int i = 0; i < m_lnSa.size(); ++i) {
        // Average of the motions added
        m_lnSa[i] = m_lnSa[i] * (n - 1) / n + motion->lnSa().at(i) / n;
        m_sa[i] = exp(m_lnSa[i]);
        sum += m_lnSa[i];
    }

    m_avgLnSa = sum / m_lnSa.size();
}

QList<Motion *> &MotionGroup::motions() {
    return m_motions;
}

const QList<Motion *> &MotionGroup::motions() const {
    return m_motions;
}
