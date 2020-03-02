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

#ifndef CONFIGURE_PLOT_DIALOG_H_
#define CONFIGURE_PLOT_DIALOG_H_

#include "AxisOptionsGroupBox.h"

#include <QDialog>

class ConfigurePlotDialog : public QDialog {
Q_OBJECT

public:
    ConfigurePlotDialog(QwtPlot *plot, QWidget *parent = 0, Qt::WindowFlags f = 0);

protected slots:

    void tryAccept();

protected:
    //! Options for the horizontal axis
    AxisOptionsGroupBox *m_xAxisOptions;

    //! Options for the vertical axis
    AxisOptionsGroupBox *m_yAxisOptions;

    QwtPlot *m_plot;
};

#endif
