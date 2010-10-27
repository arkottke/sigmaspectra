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


#ifndef PAIRED_MOTION_H_
#define PAIRED_MOTION_H_

#include "AbstractMotion.h"

class PairedMotion : AbstractMotion
{
    public:
        PairMotion( Motion & motionA, Motion & motion B );
        ~PairMotion();

        //! Scale the properties of the motion by a factor
        void scaleBy(double factor);

        //! Check if two motions are from the same event and station
        static bool isAPair(const Motion & motionA, const Motion & motionB);

    protected:
        //! First component
        Motion m_motionA;

        //! Second component
        Motion m_motionB;
}
