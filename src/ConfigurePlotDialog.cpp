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

#include "ConfigurePlotDialog.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>

ConfigurePlotDialog::ConfigurePlotDialog( QwtPlot * plot, QWidget * parent, Qt::WindowFlags f )
    : QDialog( parent, f ), m_plot( plot )
{
    // Create the dialog
    QVBoxLayout * layout = new QVBoxLayout;

    m_xAxisOptions = new AxisOptionsGroupBox( m_plot, QwtPlot::xBottom, tr("X Axis") );
   
    layout->addWidget( m_xAxisOptions );

    m_yAxisOptions = new AxisOptionsGroupBox( m_plot, QwtPlot::yLeft, tr("Y Axis") );
    
    layout->addWidget( m_yAxisOptions );
    
    // Add the buttons
    QDialogButtonBox * buttonBox = new QDialogButtonBox( 
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect( buttonBox, SIGNAL(accepted()), this, SLOT(tryAccept()));
    connect( buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget( buttonBox );

    setLayout(layout);
}

void ConfigurePlotDialog::tryAccept() 
{
    m_xAxisOptions->saveValues();
    m_yAxisOptions->saveValues();

    accept();
}

