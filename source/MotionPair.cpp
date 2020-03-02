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

#include "MotionPair.h"

#include <QDebug>
#include <QDir>

MotionPair::MotionPair(Motion *motionA, Motion *motionB)
        : AbstractMotion(), m_motionA(motionA), m_motionB(motionB) {
    m_event = m_motionA->event();
    m_station = m_motionA->station();

    m_lnSa.resize(m_period.size());
    m_sa.resize(m_period.size());

    double sum = 0;
    for (int i = 0; i < m_period.size(); ++i) {
        // Average of the motions added
        m_lnSa[i] = (m_motionA->lnSa().at(i) + m_motionB->lnSa().at(i)) / 2.;
        m_sa[i] = exp(m_lnSa[i]);
        sum += m_lnSa[i];
    }
    m_avgLnSa = sum / m_lnSa.size();
}

MotionPair::~MotionPair() {
    delete m_motionA;
    delete m_motionB;
}

QString MotionPair::name() const {
    return m_motionA->event() + QDir::separator() + m_motionA->station();
}

int MotionPair::componentCount() const {
    return 2;
}

void MotionPair::scaleBy(const double factor) {
    m_motionA->scaleBy(factor);
    m_motionB->scaleBy(factor);

    AbstractMotion::scaleBy(factor);
}

bool MotionPair::isAPair(const Motion *motionA, const Motion *motionB) {
    return (motionA->event() == motionB->event() && motionA->station() == motionB->station());
}

const Motion *MotionPair::motionA() const {
    return m_motionA;
}

const Motion *MotionPair::motionB() const {
    return m_motionB;
}
