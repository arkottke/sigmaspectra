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

#include "SuiteDialog.h"
#include "ExportDialog.h"
#include "ConfigurePlotDialog.h"

#include "MotionPair.h"

#include <QApplication>
#include <QClipboard>
#include <QGridLayout>
#include <QTableView>
#include <QPushButton>
#include <QIcon>
#include <QGroupBox>
#include <QFileDialog>
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QMenu>

#include <QDebug>

#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>

SuiteDialog::SuiteDialog(  MotionLibrary * motionLibrary, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_motionLibrary( motionLibrary )
{
    // Allow for the dialog to be maximized
    setWindowFlags(Qt::Dialog | Qt::WindowMaximizeButtonHint);

    m_selectedSuite = 0;
    m_currentPlot = 0;

    // This is the default value for Qwt
    m_zOrder = 20;

    // Create the page
    createPage();
    
    // Create the context menu
    m_plotContextMenu = new QMenu(this);
    m_plotContextMenu->addAction(QIcon(":/images/edit-copy.svg"), tr("Copy"), this, SLOT(copyPlot()));
    m_plotContextMenu->addSeparator();
    m_plotContextMenu->addAction(tr("Plot Options"), this, SLOT(configurePlot()));

    // Select the first row in the suite table
    m_suiteListTableView->selectRow(0);
}

SuiteDialog::~SuiteDialog()
{
}

void SuiteDialog::suiteSelected()
{
    m_selectedSuite = m_motionLibrary->suites().at(m_suiteListTableView->currentIndex().row());

    // Scale the suite
    m_selectedSuite->scaleMotions();

    // Enable the export button
    m_exportPushButton->setEnabled(true);

    // Plot the response spectrum
    plotSelectedSuite();
    
    // Set the new
    m_suiteTableView->setModel(m_selectedSuite);
    m_suiteTableView->resizeRowsToContents();
    m_suiteTableView->resizeColumnsToContents();
    
    connect(m_suiteTableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(motionSelected()));

    // This needs to be defined as -1 so that the previously selected motions
    // are re-colored because there are no previously selected motions.
    m_selectedMotionIndex = -1;
    m_suiteTableView->selectRow(0);
}

void SuiteDialog::motionSelected()
{
    // Division that converts between individual index and group index
    const int div = m_motionLibrary->groupSize();

    if (0 <= m_selectedMotionIndex) {
        // Reset the zOrder and color of the old index
        m_indivRespSpecCurves[m_selectedMotionIndex]->setPen(QPen(Qt::darkGray));
        m_indivRespSpecCurves[m_selectedMotionIndex]->setZ(m_zOrder);

        if (m_motionLibrary->combineComponents()) {
            m_groupedRespSpecCurves[m_selectedMotionIndex/div]->setPen(QPen(Qt::darkGray));
            m_groupedRespSpecCurves[m_selectedMotionIndex/div]->setZ(m_zOrder);
        }
    }

    m_selectedMotionIndex = m_suiteTableView->currentIndex().row();

    if (0 <= m_selectedMotionIndex) {
        // Color new motion
        m_indivRespSpecCurves[m_selectedMotionIndex]->setPen(QPen(QBrush(Qt::darkGreen), 2, Qt::SolidLine));
        m_indivRespSpecCurves[m_selectedMotionIndex]->setZ(m_zOrder+3);
        
        if (m_motionLibrary->combineComponents()) {
            m_groupedRespSpecCurves[m_selectedMotionIndex/div]->setPen(QPen(QBrush(Qt::darkGreen), 2, Qt::SolidLine));
            m_groupedRespSpecCurves[m_selectedMotionIndex/div]->setZ(m_zOrder+3);
        }
    }

    m_indivRespSpecPlot->replot();

    if (m_motionLibrary->combineComponents()) {
        m_groupedRespSpecPlot->replot();
    }

    // Plot the time histories
    const Motion * motion = m_selectedSuite->selectMotion(m_selectedMotionIndex);

    // Set the data
#if QWT_VERSION < 0x60000
    m_tsCurves[0]->setData(motion->time().data(), motion->acc().data(), motion->time().size() );
    m_tsCurves[1]->setData(motion->time().data(), motion->vel().data(), motion->time().size() );
    m_tsCurves[2]->setData(motion->time().data(), motion->disp().data(), motion->time().size() );
#else
    m_tsCurves[0]->setSamples(motion->time(), motion->acc());
    m_tsCurves[1]->setSamples(motion->time(), motion->vel());
    m_tsCurves[2]->setSamples(motion->time(), motion->disp());
#endif
}

void SuiteDialog::showTimeHistoryTab()
{
    if ( m_motionLibrary->combineComponents() ) {
        m_plotsTabWidget->setCurrentIndex(3);
    } else {
        m_plotsTabWidget->setCurrentIndex(2);
    }
}

void SuiteDialog::exportSuites()
{
    // Sort by the RMSE value -- column 2
    m_suiteListTableView->sortByColumn(2);
    
    ExportDialog dialog( m_motionLibrary->suites(), m_motionLibrary->motionPath(), this );
    dialog.exec();
}

void SuiteDialog::groupedRespSpecPointSelected(const QPoint & point)
{
    // Find the index of the closest curve
    double distance;
    double minDistance = -1;
    int minIndex = 0;

    for ( int i = 0; i < m_groupedRespSpecCurves.size()-6; ++i ) {
        if ( !m_groupedRespSpecCurves.at(i)->plot() )
            // Continue if not attached to the plot
            continue;

        m_groupedRespSpecCurves.at(i)->closestPoint( point, &distance);

        if ( minDistance < 0 || distance < minDistance ) {
            minDistance = distance;
            minIndex = i;
        }
    }

    // Select this item from the table
    const int mult = m_motionLibrary->groupSize();
    m_suiteTableView->selectRow(mult*minIndex);
}

void SuiteDialog::indivRespSpecPointSelected(const QPoint & point)
{
    // Find the index of the closest curve
    double distance;
    double minDistance = -1;
    int minIndex = 0;

    for ( int i = 0; i < m_indivRespSpecCurves.size()-6; ++i ) {
        if ( !m_indivRespSpecCurves.at(i)->plot() )
            // Continue if not attached to the plot
            continue;

        m_indivRespSpecCurves.at(i)->closestPoint( point, &distance);

        if ( minDistance < 0 || distance < minDistance ) {
            minDistance = distance;
            minIndex = i;
        }
    }

    // Select this item from the table
    m_suiteTableView->selectRow(minIndex);
}

void SuiteDialog::showPlotContextMenu(const QPoint & point)
{
    QwtPlot * plot = qobject_cast<QwtPlot*>(sender());

    if ( plot ) {
        m_currentPlot = plot;
        m_plotContextMenu->popup( plot->mapToGlobal(point) );
    }
}

void SuiteDialog::copyPlot()
{
    if ( m_currentPlot ) {
        if ( m_currentPlot == m_indivRespSpecPlot ) {
            // Reset the zOrder and color of the old index
            m_indivRespSpecCurves[m_selectedMotionIndex]->setPen(QPen(Qt::darkGray));
            m_indivRespSpecCurves[m_selectedMotionIndex]->setZ(m_zOrder);

            m_indivRespSpecPlot->replot();

        } else if ( m_currentPlot == m_groupedRespSpecPlot ) {
            // Division that converts between individual index and group index
            const int div = m_motionLibrary->groupSize();

            m_groupedRespSpecCurves[m_selectedMotionIndex/div]->setPen(QPen(Qt::darkGray));
            m_groupedRespSpecCurves[m_selectedMotionIndex/div]->setZ(m_zOrder);
            
            m_groupedRespSpecPlot->replot();
        }

        // Set the clilpboard image
        QClipboard * clipboard = QApplication::clipboard();
        clipboard->setPixmap(QPixmap::grabWidget(m_currentPlot));

        if ( m_currentPlot == m_indivRespSpecPlot ) {
            // Reset the zOrder and color of the old index
            m_indivRespSpecCurves[m_selectedMotionIndex]->setPen(QPen(QBrush(Qt::darkGreen), 2, Qt::SolidLine));
            m_indivRespSpecCurves[m_selectedMotionIndex]->setZ(m_zOrder+3);


            m_indivRespSpecPlot->replot();

        } else if ( m_currentPlot == m_groupedRespSpecPlot ) {
            // Division that converts between individual index and group index
            const int div = m_motionLibrary->groupSize();

            m_groupedRespSpecCurves[m_selectedMotionIndex/div]->setPen(QPen(QBrush(Qt::darkGreen), 2, Qt::SolidLine));
            m_groupedRespSpecCurves[m_selectedMotionIndex/div]->setZ(m_zOrder+3);
            
            m_groupedRespSpecPlot->replot();
        }
    }
}

void SuiteDialog::configurePlot()
{
    if ( m_currentPlot ) {
        ConfigurePlotDialog dialog( m_currentPlot, this );

        if (dialog.exec())
            m_currentPlot->replot();
    }
}

void SuiteDialog::plotSelectedSuite()
{
    // 
    // Grouped response spectra curves
    //
    if (m_motionLibrary->combineComponents()) {
        // Set the data for the suite spectra
        int i = 0;

        for (i = 0; i < m_selectedSuite->motions().size(); ++i) {
#if QWT_VERSION < 0x60000
            m_groupedRespSpecCurves[i]->setData(
                    m_selectedSuite->motions().at(i)->period().data(), 
                    m_selectedSuite->motions().at(i)->sa().data(), 
                    m_selectedSuite->motions().at(i)->period().size() );
#else
            m_groupedRespSpecCurves[i]->setSamples(
                        m_selectedSuite->motions().at(i)->period(),
                        m_selectedSuite->motions().at(i)->sa());
#endif
            m_groupedRespSpecCurves[i]->setPen(QPen(Qt::darkGray));
            m_groupedRespSpecCurves[i]->setZ(m_zOrder);
        }

        // Set the average response spectrum
#if QWT_VERSION < 0x60000
        m_groupedRespSpecCurves[i]->setData(
                m_motionLibrary->period().data(),
                m_selectedSuite->avgSa().data(),
                m_motionLibrary->period().size());
#else
        m_groupedRespSpecCurves[i]->setSamples(
                    m_motionLibrary->period(),
                    m_selectedSuite->avgSa());
#endif
        m_groupedRespSpecCurves[i]->setPen(QPen(QBrush(Qt::blue), 2, Qt::SolidLine));
        m_groupedRespSpecCurves[i]->setZ(m_zOrder+2);

#if QWT_VERSION < 0x60000
        m_groupedRespSpecCurves[++i]->setData(
                m_motionLibrary->period().data(),
                m_selectedSuite->avgPlusStd().data(),
                m_motionLibrary->period().size());
#else
        m_groupedRespSpecCurves[++i]->setSamples(
                    m_motionLibrary->period(),
                    m_selectedSuite->avgPlusStd());
#endif
        m_groupedRespSpecCurves[i]->setPen(QPen(QBrush(Qt::blue), 2, Qt::DashLine));
        m_groupedRespSpecCurves[i]->setZ(m_zOrder+2);

#if QWT_VERSION < 0x60000
        m_groupedRespSpecCurves[++i]->setData(
                m_motionLibrary->period().data(),
                m_selectedSuite->avgMinusStd().data(),
                m_motionLibrary->period().size());
#else
        m_groupedRespSpecCurves[++i]->setSamples(
                    m_motionLibrary->period(),
                    m_selectedSuite->avgMinusStd());
#endif
        m_groupedRespSpecCurves[i]->setPen(QPen(QBrush(Qt::blue), 2, Qt::DashLine));
        m_groupedRespSpecCurves[i]->setZ(m_zOrder+2);

        // Set the data target curves
#if QWT_VERSION < 0x60000
        m_groupedRespSpecCurves[++i]->setData(
                m_motionLibrary->period().data(),
                m_motionLibrary->targetSa().data(),
                m_motionLibrary->period().size());
#else
        m_groupedRespSpecCurves[++i]->setSamples(
                    m_motionLibrary->period(),
                    m_motionLibrary->targetSa());
#endif
        m_groupedRespSpecCurves[i]->setPen(QPen(QBrush(Qt::red), 2, Qt::SolidLine));
        m_groupedRespSpecCurves[i]->setZ(m_zOrder+1);

#if QWT_VERSION < 0x60000
        m_groupedRespSpecCurves[++i]->setData(
                m_motionLibrary->period().data(),
                m_motionLibrary->targetSaPlusStd().data(),
                m_motionLibrary->period().size());
#else
        m_groupedRespSpecCurves[++i]->setSamples(
                m_motionLibrary->period(),
                m_motionLibrary->targetSaPlusStd());
#endif
        m_groupedRespSpecCurves[i]->setPen(QPen(QBrush(Qt::red), 2, Qt::DashLine));
        m_groupedRespSpecCurves[i]->setZ(m_zOrder+1);

#if QWT_VERSION < 0x60000
        m_groupedRespSpecCurves[++i]->setData(
                m_motionLibrary->period().data(),
                m_motionLibrary->targetSaMinusStd().data(),
                m_motionLibrary->period().size());
#else
        m_groupedRespSpecCurves[++i]->setSamples(
                m_motionLibrary->period(),
                m_motionLibrary->targetSaMinusStd());
#endif
        m_groupedRespSpecCurves[i]->setPen(QPen(QBrush(Qt::red), 2, Qt::DashLine));
        m_groupedRespSpecCurves[i]->setZ(m_zOrder+1);

        m_groupedRespSpecPlot->replot();
    }

    // 
    // Individual response spectra curves
    //
    // Set the data for the suite spectra
    int i = 0;
    
    for (i = 0; i < m_selectedSuite->rowCount(); ++i ) {
        const Motion * motion = m_selectedSuite->selectMotion(i);

#if QWT_VERSION < 0x60000
        m_indivRespSpecCurves[i]->setData(
                motion->period().data(),
                motion->sa().data(),
                motion->period().size() );
#else
        m_indivRespSpecCurves[i]->setSamples(
                motion->period(),
                motion->sa());
#endif
        m_indivRespSpecCurves[i]->setPen(QPen(Qt::darkGray));
        m_indivRespSpecCurves[i]->setZ(m_zOrder);
    }
    
    // Set the average response spectrum
#if QWT_VERSION < 0x60000
    m_indivRespSpecCurves[i]->setData(
            m_motionLibrary->period().data(),
            m_selectedSuite->avgSa().data(),
            m_motionLibrary->period().size());
#else
    m_indivRespSpecCurves[i]->setSamples(
                m_motionLibrary->period(),
                m_selectedSuite->avgSa());
#endif
    m_indivRespSpecCurves[i]->setPen(QPen(QBrush(Qt::blue), 2, Qt::SolidLine));
    m_indivRespSpecCurves[i]->setZ(m_zOrder+2);
    
#if QWT_VERSION < 0x60000
    m_indivRespSpecCurves[++i]->setData(
            m_motionLibrary->period().data(),
            m_selectedSuite->avgPlusStd().data(),
            m_motionLibrary->period().size());
#else
    m_indivRespSpecCurves[++i]->setSamples(
            m_motionLibrary->period(),
            m_selectedSuite->avgPlusStd());
#endif
    m_indivRespSpecCurves[i]->setPen(QPen(QBrush(Qt::blue), 2, Qt::DashLine));
    m_indivRespSpecCurves[i]->setZ(m_zOrder+2);
    
#if QWT_VERSION < 0x60000
    m_indivRespSpecCurves[++i]->setData(
            m_motionLibrary->period().data(),
            m_selectedSuite->avgMinusStd().data(),
            m_motionLibrary->period().size());
#else
    m_indivRespSpecCurves[++i]->setSamples(
            m_motionLibrary->period(),
            m_selectedSuite->avgMinusStd());
#endif
    m_indivRespSpecCurves[i]->setPen(QPen(QBrush(Qt::blue), 2, Qt::DashLine));
    m_indivRespSpecCurves[i]->setZ(m_zOrder+2);

    // Set the data target curves
#if QWT_VERSION < 0x60000
    m_indivRespSpecCurves[++i]->setData(
            m_motionLibrary->period().data(),
            m_motionLibrary->targetSa().data(),
            m_motionLibrary->period().size());
#else
    m_indivRespSpecCurves[++i]->setSamples(
            m_motionLibrary->period(),
            m_motionLibrary->targetSa());
#endif
    m_indivRespSpecCurves[i]->setPen(QPen(QBrush(Qt::red), 2, Qt::SolidLine));
    m_indivRespSpecCurves[i]->setZ(m_zOrder+1);
    
#if QWT_VERSION < 0x60000
    m_indivRespSpecCurves[++i]->setData(
            m_motionLibrary->period().data(),
            m_motionLibrary->targetSaPlusStd().data(),
            m_motionLibrary->period().size());
#else
    m_indivRespSpecCurves[++i]->setSamples(
            m_motionLibrary->period(),
            m_motionLibrary->targetSaPlusStd());
#endif
    m_indivRespSpecCurves[i]->setPen(QPen(QBrush(Qt::red), 2, Qt::DashLine));
    m_indivRespSpecCurves[i]->setZ(m_zOrder+1);
    
#if QWT_VERSION < 0x60000
    m_indivRespSpecCurves[++i]->setData(
            m_motionLibrary->period().data(),
            m_motionLibrary->targetSaMinusStd().data(),
            m_motionLibrary->period().size());
#else
    m_indivRespSpecCurves[++i]->setSamples(
            m_motionLibrary->period(),
            m_motionLibrary->targetSaMinusStd());
#endif
    m_indivRespSpecCurves[i]->setPen(QPen(QBrush(Qt::red), 2, Qt::DashLine));
    m_indivRespSpecCurves[i]->setZ(m_zOrder+1);

    m_indivRespSpecPlot->replot();

    // Plot the standard deviations
#if QWT_VERSION < 0x60000
    m_stdCurves[0]->setData(
            m_motionLibrary->period().data(),
            m_motionLibrary->targetLnStd().data(),
            m_motionLibrary->period().size());
#else
    m_stdCurves[0]->setSamples(
            m_motionLibrary->period(),
            m_motionLibrary->targetLnStd());
#endif
    m_stdCurves[0]->setPen(QPen(QBrush(Qt::blue), 2, Qt::SolidLine));
    
#if QWT_VERSION < 0x60000
    m_stdCurves[1]->setData(
            m_motionLibrary->period().data(),
            m_selectedSuite->lnStd().data(),
            m_motionLibrary->period().size());
#else
    m_stdCurves[1]->setSamples(
                m_motionLibrary->period(),
                m_selectedSuite->lnStd());
#endif
    m_stdCurves[1]->setPen(QPen(QBrush(Qt::red), 2, Qt::SolidLine));
}

void SuiteDialog::createPage()
{
    QGridLayout * layout;

    // Suite list group box
    layout = new QGridLayout;

    m_suiteListTableView = new QTableView;
    m_suiteListTableView->setModel(m_motionLibrary);
    m_suiteListTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_suiteListTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_suiteListTableView->resizeColumnsToContents();
    m_suiteListTableView->resizeRowsToContents();
    m_suiteListTableView->setSortingEnabled(true);
    m_suiteListTableView->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );

    connect(m_suiteListTableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(suiteSelected()));
    layout->addWidget(m_suiteListTableView, 0, 0);

    QGroupBox * suiteListGroupBox = new QGroupBox(tr("Suite List"));
    suiteListGroupBox->setLayout(layout);

    // Motion list group box
    layout = new QGridLayout;

    m_suiteTableView = new QTableView;
    m_suiteTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_suiteTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_suiteTableView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    connect( m_suiteTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showTimeHistoryTab()));
    layout->addWidget(m_suiteTableView, 0, 0 );

    QGroupBox * suiteGroupBox = new QGroupBox(tr("Motions of Selected Suite"));
    suiteGroupBox->setLayout(layout);

    // Create the plots
    createTabWidget();
    m_plotsTabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); 

    // Create button row
    QGridLayout * buttonRowLayout = new QGridLayout;
    buttonRowLayout->setColumnStretch(0,1);

    m_exportPushButton = new QPushButton(QIcon(":/images/document-save-as.svg"), tr("Export Suites..."));
    connect( m_exportPushButton, SIGNAL(clicked()), SLOT(exportSuites()));
    buttonRowLayout->addWidget( m_exportPushButton, 0, 1);

    QPushButton * closePushButton = new QPushButton(tr("Close"));
    connect( closePushButton, SIGNAL(clicked()), SLOT(close()));
    buttonRowLayout->addWidget( closePushButton, 0, 2);

    // Set the layout of the entire dialog
    layout = new QGridLayout;

    layout->addWidget( suiteListGroupBox, 0, 0 );
    layout->addWidget( m_plotsTabWidget, 0, 1 );
    layout->addWidget( suiteGroupBox, 1, 0, 1, 2 );
    layout->addLayout( buttonRowLayout, 2, 0, 1, 2);

    setLayout(layout);
}

void SuiteDialog::createTabWidget()
{
    m_plotsTabWidget = new QTabWidget;

    // Average response spectrum plot
    if ( m_motionLibrary->combineComponents() ) {
        m_groupedRespSpecPlot = new QwtPlot;
        configurePlot(m_groupedRespSpecPlot, tr("Period (s)"), tr("Spectral Accel. (g)"));
        m_groupedRespSpecPlot->setAxisScaleEngine( QwtPlot::xBottom, new QwtLog10ScaleEngine);
        m_groupedRespSpecPlot->setContextMenuPolicy(Qt::CustomContextMenu);

        connect( m_groupedRespSpecPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showPlotContextMenu(QPoint)));

#if QWT_VERSION < 0x60000
        QwtPlotPicker * picker = new QwtPlotPicker(
                QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection,
                QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, m_groupedRespSpecPlot->canvas());
#else
        QwtPlotPicker * picker = new QwtPlotPicker(
                    QwtPlot::xBottom, QwtPlot::yLeft,
                    QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, m_groupedRespSpecPlot->canvas());
        picker->setStateMachine(new QwtPickerDragPointMachine());
#endif

        connect(picker, SIGNAL(appended(QPoint)), this, SLOT(groupedRespSpecPointSelected(QPoint)));


        // Number of motion pairs
        int count = m_motionLibrary->suiteSize();
        // Add the median and +/- sigma curves for both the target and the suite
        count += 6;

        while ( m_groupedRespSpecCurves.size() < count ) {
            m_groupedRespSpecCurves << new QwtPlotCurve;
            m_groupedRespSpecCurves.last()->attach(m_groupedRespSpecPlot);
        }

        m_plotsTabWidget->addTab( m_groupedRespSpecPlot, tr("Grouped Response Spectra") );
    } else {
        m_groupedRespSpecPlot = 0; 
    }

    // Response spectrum
    m_indivRespSpecPlot = new QwtPlot;
    configurePlot(m_indivRespSpecPlot, tr("Period (s)"), tr("Spectral Accel. (g)"));
    m_indivRespSpecPlot->setAxisScaleEngine( QwtPlot::xBottom, new QwtLog10ScaleEngine);
    m_indivRespSpecPlot->setContextMenuPolicy(Qt::CustomContextMenu);

    connect( m_indivRespSpecPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showPlotContextMenu(QPoint)));
    
#if QWT_VERSION < 0x60000
    QwtPlotPicker * picker = new QwtPlotPicker(
            QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection,
            QwtPicker::CrossRubberBand, QwtPicker::AlwaysOn, m_indivRespSpecPlot->canvas());
    picker->setStateMachine(new QwtPickerTrackerMachine());
#else
    QwtPlotPicker * picker = new QwtPlotPicker(
                QwtPlot::xBottom, QwtPlot::yLeft,
                QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, m_indivRespSpecPlot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());
#endif

    connect( picker, SIGNAL(appended(QPoint)), this, SLOT(indivRespSpecPointSelected(QPoint)));


    // FIXME Add legend!
//    // Legend
//    QwtLegend* legend = new QwtLegend;
//    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
//    m_indivRespSpecPlot->insertLegend(legend, QwtPlot::BottomLegend);
//
//    // Add the generic curves to the legend
//    QwtPlotCurve curveA(tr("Unselected Motion"));
//    curveA.setPen(QPen(Qt::darkGray));
//    curveA.updateLegend(legend);
//
//    QwtPlotCurve curveB(tr("Selected Motion"));
//    curveB.setPen(QPen(QBrush(Qt::darkGreen), 2));
//    curveB.updateLegend(legend);
//
//    QwtPlotCurve curveC(tr("Median of Suite"));
//    curveA.setPen(QPen(Qt::blue), 2);
//    curveA.updateLegend(legend);
//
//    QwtPlotCurve curveB(tr("Selected Motion"));
//    curveB.setPen(QPen(QBrush(Qt::darkGreen), 2));
//    curveB.updateLegend(legend);

    // Create curves for the plot -- for each of the motions and the median,
    // plus sigma, and minus sigma for both the target and the suite.
    m_motionCount = m_motionLibrary->suites().first()->rowCount();
    // 6 more curves for the median and standard deviations
    int count = m_motionCount + 6;

    while ( m_indivRespSpecCurves.size() < count ) {
        m_indivRespSpecCurves << new QwtPlotCurve;
        m_indivRespSpecCurves.last()->attach(m_indivRespSpecPlot);
    }

    m_plotsTabWidget->addTab( m_indivRespSpecPlot, tr("Individual Response Spectra") );
    
    // Standard deviation
    QwtPlot * stdPlot =  new QwtPlot;
    configurePlot(stdPlot, tr("Period (s)"), tr("Std. (%1_ln)").arg(QChar(0x03C3)));
    stdPlot->setAutoReplot(true);
    stdPlot->setAxisScaleEngine( QwtPlot::xBottom, new QwtLog10ScaleEngine);
    stdPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::IncludeReference, true);
    stdPlot->setContextMenuPolicy(Qt::CustomContextMenu);

    connect( stdPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showPlotContextMenu(QPoint)));
    

#if QWT_VERSION < 0x60000
    picker = new QwtPlotPicker(
            QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection,
            QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, stdPlot->canvas());
#else
    picker = new QwtPlotPicker(
                QwtPlot::xBottom, QwtPlot::yLeft,
                QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, stdPlot->canvas());
    picker->setStateMachine(new QwtPickerTrackerMachine());
#endif

    m_plotsTabWidget->addTab( stdPlot, tr("Standard Deviation") );

    while (m_stdCurves.size() < 2) {
        m_stdCurves << new QwtPlotCurve;
        m_stdCurves.last()->attach(stdPlot);
    }

    // Time histories
    QVBoxLayout * layout = new QVBoxLayout;

    // Acceleration plot
    QwtPlot * accPlot = new QwtPlot;
    accPlot->setAutoReplot(true);
    configurePlot(accPlot, "", tr("Accel. (g)"));
    accPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Symmetric, true);
    accPlot->setAxisTitle( QwtPlot::yLeft, tr("Accel. (g)") );
    accPlot->setContextMenuPolicy(Qt::CustomContextMenu);

    connect( accPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showPlotContextMenu(QPoint)));

#if QWT_VERSION < 0x60000
    picker = new QwtPlotPicker(
            QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection,
            QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, accPlot->canvas());
#else
    picker = new QwtPlotPicker(
                QwtPlot::xBottom, QwtPlot::yLeft,
                QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, accPlot->canvas());
    picker->setStateMachine(new QwtPickerTrackerMachine());
#endif

    m_tsCurves << new QwtPlotCurve;
    m_tsCurves.last()->attach(accPlot);
    m_tsCurves.last()->setPen(QPen(Qt::blue));

    layout->addWidget(accPlot);

    // Velocity plot
    QwtPlot * velPlot = new QwtPlot;
    velPlot->setAutoReplot(true);
    velPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Symmetric, true);
    configurePlot(velPlot, "", tr("Vel. (cm/s)"));
    velPlot->setContextMenuPolicy(Qt::CustomContextMenu);

    connect( velPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showPlotContextMenu(QPoint)));
    
#if QWT_VERSION < 0x60000
    picker = new QwtPlotPicker(
            QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection,
            QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, velPlot->canvas());
#else
    picker = new QwtPlotPicker(
                QwtPlot::xBottom, QwtPlot::yLeft,
                QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, velPlot->canvas());
    picker->setStateMachine(new QwtPickerTrackerMachine());
#endif

    m_tsCurves << new QwtPlotCurve;
    m_tsCurves.last()->attach(velPlot);
    m_tsCurves.last()->setPen(QPen(Qt::blue));

    layout->addWidget(velPlot);

    // Displacment plot
    QwtPlot * dispPlot = new QwtPlot;
    dispPlot->setAutoReplot(true);
    dispPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Symmetric, true);    
    configurePlot(dispPlot, tr("Time (s)"), tr("Disp. (cm)"));
    dispPlot->setContextMenuPolicy(Qt::CustomContextMenu);

    connect( dispPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showPlotContextMenu(QPoint)));
    
#if QWT_VERSION < 0x60000
    picker = new QwtPlotPicker(
            QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection,
            QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, dispPlot->canvas());
#else
    picker = new QwtPlotPicker(
                QwtPlot::xBottom, QwtPlot::yLeft,
                QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, dispPlot->canvas());
    picker->setStateMachine(new QwtPickerTrackerMachine());
#endif

    m_tsCurves << new QwtPlotCurve;
    m_tsCurves.last()->attach(dispPlot);
    m_tsCurves.last()->setPen(QPen(Qt::blue));

    layout->addWidget(dispPlot);

    QFrame * tsFrame = new QFrame;
    // Allow the frame to be shrinked vertically
    tsFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

    tsFrame->setLayout(layout);

    m_plotsTabWidget->addTab( tsFrame, tr("Time Series"));
}


void SuiteDialog::configurePlot(QwtPlot* plot, const QString& xLabel, const QString& yLabel)
{
    // Label the axes
    QFont font = QApplication::font();
    plot->setAxisFont(QwtPlot::xBottom, font);
    plot->setAxisFont(QwtPlot::yLeft, font);

    font.setBold(true);
    QwtText text;
    text.setFont(font);

    if (!xLabel.isEmpty()) {
        text.setText(xLabel);
        plot->setAxisTitle(QwtPlot::xBottom, text);
    }

    if (!yLabel.isEmpty()) {
        text.setText(yLabel);
        plot->setAxisTitle(QwtPlot::yLeft, text);
    }
}
