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

#include "AxisOptionsGroupBox.h"

#include <QGridLayout>
#include <QLabel>

#include <qwt_scale_engine.h>

AxisOptionsGroupBox::AxisOptionsGroupBox(QwtPlot *plot, int axisId,
                                         const QString &title, QWidget *parent)
        : QGroupBox(title, parent), m_plot(plot), m_axisId(axisId) {
    QGridLayout *layout = new QGridLayout;

    //
    // Create the widgets
    //

    // Spacing combo box
    layout->addWidget(new QLabel(tr("Spacing:")), 0, 0);

    m_spacingComboBox = new QComboBox;
    m_spacingComboBox->addItem(tr("Linear"));
    m_spacingComboBox->addItem(tr("Log10"));

    layout->addWidget(m_spacingComboBox, 0, 1);

    // Automatic scaling of axis
    m_autoCheckBox = new QCheckBox(tr("Automatic scaling of axis"));

    layout->addWidget(m_autoCheckBox, 1, 0);

    // Minimum value
    layout->addWidget(new QLabel(tr("Minimum value:")), 2, 0);

    m_minLineEdit = new QLineEdit;
    m_minLineEdit->setValidator(new QDoubleValidator(m_minLineEdit));

    connect(m_autoCheckBox, SIGNAL(toggled(bool)), m_minLineEdit,
            SLOT(setDisabled(bool)));

    layout->addWidget(m_minLineEdit, 2, 1);

    // Maximum value
    layout->addWidget(new QLabel(tr("Maximum value:")), 3, 0);

    m_maxLineEdit = new QLineEdit;
    m_maxLineEdit->setValidator(new QDoubleValidator(m_maxLineEdit));

    connect(m_autoCheckBox, SIGNAL(toggled(bool)), m_maxLineEdit,
            SLOT(setDisabled(bool)));

    layout->addWidget(m_maxLineEdit, 3, 1);

    setLayout(layout);

    //
    // Load the values
    //
    if (dynamic_cast<const QwtLinearScaleEngine *>(
            m_plot->axisScaleEngine(m_axisId))) {
        m_spacingComboBox->setCurrentIndex(0);
    } else {
        m_spacingComboBox->setCurrentIndex(1);
    }

    if (m_plot->axisAutoScale(m_axisId)) {
        m_autoCheckBox->setChecked(true);
    } else {
        m_autoCheckBox->setChecked(false);
    }

    const QwtScaleDiv &scaleDiv = m_plot->axisScaleDiv(m_axisId);
    m_minLineEdit->setText(QString::number(scaleDiv.lowerBound()));
    m_maxLineEdit->setText(QString::number(scaleDiv.upperBound()));
}

void AxisOptionsGroupBox::saveValues() {
    QwtScaleEngine *oldEngine = m_plot->axisScaleEngine(m_axisId);
    QwtScaleEngine *newEngine;

    if (m_spacingComboBox->currentIndex() == 0) {
        newEngine = new QwtLinearScaleEngine;
    } else {
        newEngine = new QwtLogScaleEngine(10);
    }

    newEngine->setAttributes(oldEngine->attributes());

    // Deletes the old engine
    m_plot->setAxisScaleEngine(m_axisId, newEngine);

    // Set the axis range
    if (m_autoCheckBox->isChecked()) {
        m_plot->setAxisAutoScale(m_axisId);
    } else {
        m_plot->setAxisScale(m_axisId, m_minLineEdit->text().toDouble(),
                             m_maxLineEdit->text().toDouble());
    }
}
