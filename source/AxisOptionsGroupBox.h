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

#ifndef AXIS_OPTIONS_GROUP_BOX_H_
#define AXIS_OPTIONS_GROUP_BOX_H_

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>

#include <qwt_plot.h>

class AxisOptionsGroupBox : public QGroupBox {
Q_OBJECT

public:
    AxisOptionsGroupBox(const QString &title, QWidget *parent = 0);

    AxisOptionsGroupBox(QwtPlot *plot, int axisId, const QString &title, QWidget *parent = 0);

    //! Save values to the plot
    void saveValues();

private:
    QComboBox *m_spacingComboBox;

    QCheckBox *m_autoCheckBox;

    QLineEdit *m_minLineEdit;
    QLineEdit *m_maxLineEdit;

    QwtPlot *m_plot;

    const int m_axisId;
};

#endif
