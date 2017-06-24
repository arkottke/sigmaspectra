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

#include "SuiteDialog.h"
#include "ConfigurePlotDialog.h"
#include "ExportDialog.h"

#include "MotionPair.h"

#include <QApplication>
#include <QClipboard>
#include <QGridLayout>
#include <QMenu>
#include <QPushButton>


#include <qwt_legend.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_engine.h>

SuiteDialog::SuiteDialog(MotionLibrary *motionLibrary, QWidget *parent,
                         Qt::WindowFlags f)
        : QDialog(parent, f), m_motionLibrary(motionLibrary) {
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
    m_plotContextMenu->addAction(QIcon(":/images/edit-copy.svg"), tr("Copy"),
                                 this, SLOT(copyPlot()));
    m_plotContextMenu->addSeparator();
    m_plotContextMenu->addAction(tr("Plot Options"), this, SLOT(configurePlot()));

    // Select the first row in the suite table
    m_suiteListTableView->selectRow(0);
}

SuiteDialog::~SuiteDialog() {}

void SuiteDialog::suiteSelected() {
    m_selectedSuite = m_motionLibrary->suites().at(m_suiteListTableView->currentIndex().row());

    // Scale the suite
    m_selectedSuite->scaleMotions();

    // Enable the export button
    m_exportPushButton->setEnabled(true);

    // Plot the response spectrum
    plotSelectedSuite();

    // Set the new
    m_suiteTableView->setModel(m_selectedSuite);
    m_suiteTableView->resizeColumnsToContents();
    m_suiteTableView->resizeRowsToContents();

    connect(m_suiteTableView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            SLOT(motionSelected()));

    // This needs to be defined as -1 so that the previously selected motions
    // are re-colored because there are no previously selected motions.
    m_selectedCurves.clear();
    m_suiteTableView->selectRow(0);
}


QwtPlotCurve *SuiteDialog::createCurve(QwtPlot *plot, QString key, QPen pen, double zOrder) {
    QwtPlotCurve *curve = new QwtPlotCurve;
    curve->setPen(pen);
    curve->setZ(zOrder);
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->attach(plot);
    m_curves.insertMulti(key, curve);
    return curve;
}


void SuiteDialog::motionSelected() {
    // Division that converts between individual index and group index
    const int div = m_motionLibrary->groupSize();
    for (QwtPlotCurve *curve : m_selectedCurves) {
        resetCurve(curve);
    }

    m_selectedCurves.clear();
    int index = m_suiteTableView->currentIndex().row();

    if (0 <= index) {
        // Color new motion
        m_selectedCurves << m_curves.values("indivRespSpec")[index];
        if (m_motionLibrary->combineComponents()) {
            m_selectedCurves << m_curves.values("groupedRespSpec")[index / div];
        }

        for (QwtPlotCurve *curve : m_selectedCurves) {
            highlightCurve(curve);
        }
    }

    // FIXME - needed?
    // m_indivRespSpecPlot->replot();
    //    if (m_motionLibrary->combineComponents()) {
    //        m_groupedRespSpecPlot->replot();
    //    }

    // Plot the time histories
    const Motion *motion = m_selectedSuite->selectMotion(index);

    // Set the data
    QList<QwtPlotCurve *> curves = m_curves.values("timeSeries");
    curves.at(0)->setSamples(motion->time(), motion->acc());
    curves.at(1)->setSamples(motion->time(), motion->vel());
    curves.at(2)->setSamples(motion->time(), motion->disp());
}

void SuiteDialog::showTimeHistoryTab() {
    if (m_motionLibrary->combineComponents()) {
        m_plotsTabWidget->setCurrentIndex(3);
    } else {
        m_plotsTabWidget->setCurrentIndex(2);
    }
}

void SuiteDialog::exportSuites() {
    // Sort by the RMSE value -- column 2
    m_suiteListTableView->sortByColumn(2);

    ExportDialog dialog(
            m_motionLibrary->suites(), m_motionLibrary->motionPath(), this);
    dialog.exec();
}


int closestCurveIndex(const QPoint &point, QList<QwtPlotCurve *> curves) {
    // Find the index of the closest curve
    double distance;
    double minDistance = -1;
    int minIndex = 0;
    int index = 0;
    for (QwtPlotCurve *curve : curves) {
        if (!curve->plot()) {
            // Continue if not attached to the plot
            continue;
        }
        curve->closestPoint(point, &distance);
        if (minDistance < 0 || distance < minDistance) {
            minDistance = distance;
            minIndex = index;
        }
        index += 1;
    }
    return minIndex;
}

void SuiteDialog::groupedRespSpecPointSelected(const QPoint &point) {
    int index = closestCurveIndex(point, m_curves.values("groupedRespSpec"));
    // Scale the index by the group size
    const int mult = m_motionLibrary->groupSize();
    m_suiteTableView->selectRow(mult * index);
}

void SuiteDialog::indivRespSpecPointSelected(const QPoint &point) {
    int index = closestCurveIndex(point, m_curves.values("indivRespSpec"));
    m_suiteTableView->selectRow(index);
}

void SuiteDialog::showPlotContextMenu(const QPoint &point) {
    QwtPlot *plot = static_cast<QwtPlot *>(sender());
    if (plot) {
        m_currentPlot = plot;
        m_plotContextMenu->popup(plot->mapToGlobal(point));
    }
}

void SuiteDialog::copyPlot() {
    if (m_currentPlot) {
        for (QwtPlotCurve *curve : m_selectedCurves) {
            resetCurve(curve);
        }
        // Set the clilpboard image
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setPixmap(QPixmap::grabWidget(m_currentPlot));

        for (QwtPlotCurve *curve : m_selectedCurves) {
            highlightCurve(curve);
        }
    }
}

void SuiteDialog::configurePlot() {
    if (m_currentPlot) {
        ConfigurePlotDialog dialog(m_currentPlot, this);

        if (dialog.exec()) {
            m_currentPlot->replot();
        }
    }
}

void SuiteDialog::plotSelectedSuite() {
    QList<QwtPlotCurve *> curves;
    int i = 0;
    //
    // Grouped response spectra curves
    //
    if (m_motionLibrary->combineComponents()) {
        // Set the data for the suite spectra
        curves = m_curves.values("groupedRespSpec");
        i = 0;
        for (AbstractMotion *motion : m_selectedSuite->motions()) {
            curves[i]->setSamples(motion->period(), motion->sa());
            ++i;
        }
// Set the average response spectrum
        curves = m_curves.values("groupedRespSpecStats");
        i = 0;
        for (double eps : {0, -1, 1}) {
            curves[i]->setSamples(m_motionLibrary->period(),
                                  m_selectedSuite->fractile(eps));
            ++i;
        }
// Set the data target curves
        for (double eps : {0, -1, 1}) {
            curves[i]->setSamples(m_motionLibrary->period(),
                                  m_motionLibrary->targetFracile(eps));
            ++i;
        }
        // FIXME m_groupedRespSpecPlot->replot();
    }

    //
    // Individual response spectra curves
    //
    // Set the data for the suite spectra
    curves = m_curves.values("indivRespSpec");
    i = 0;
    for (AbstractMotion *absMotion : m_selectedSuite->motions()) {
        if (MotionPair *motionPair = dynamic_cast<MotionPair *>(absMotion)) {
            for (const Motion *motion: {motionPair->motionA(), motionPair->motionB()}) {
                curves[i]->setSamples(motion->period(), motion->sa());
                ++i;
            }
        } else {
            Motion *motion = dynamic_cast<Motion *>(absMotion);
            curves[i]->setSamples(motion->period(), motion->sa());
            ++i;
        }
    }

// Set the average response spectrum
    curves = m_curves.values("indivRespSpecStats");
    i = 0;
    for (double eps : {0, -1, 1}) {
        curves[i]->setSamples(m_motionLibrary->period(),
                              m_selectedSuite->fractile(eps));
        ++i;
    }

// Set the data target curves
    for (double eps : {0, -1, 1}) {
        curves[i]->setSamples(m_motionLibrary->period(),
                              m_motionLibrary->targetFracile(eps));
        ++i;
    }

    // m_indivRespSpecPlot->replot();

    // FIXME setRenderHint(QwtPlotItem::RenderAntialiased);

// Plot the standard deviations
    curves = m_curves.values("stdev");
    curves[0]->setSamples(m_motionLibrary->period(),
                          m_motionLibrary->targetLnStd());
    curves[1]->setSamples(m_motionLibrary->period(),
                          m_selectedSuite->lnStd());
}

void SuiteDialog::createPage() {
    QGridLayout *layout;

    // Suite list group box
    layout = new QGridLayout;

    m_suiteListTableView = new QTableView;
    m_suiteListTableView->setModel(m_motionLibrary);
    m_suiteListTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_suiteListTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_suiteListTableView->resizeColumnsToContents();
    m_suiteListTableView->resizeRowsToContents();
    m_suiteListTableView->setSortingEnabled(true);
    m_suiteListTableView->setSizePolicy(QSizePolicy::Fixed,
                                        QSizePolicy::Expanding);

    connect(m_suiteListTableView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
            SLOT(suiteSelected()));
    layout->addWidget(m_suiteListTableView, 0, 0);

    QGroupBox *suiteListGroupBox = new QGroupBox(tr("Suite List"));
    suiteListGroupBox->setLayout(layout);

    // Motion list group box
    layout = new QGridLayout;

    m_suiteTableView = new QTableView;
    m_suiteTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_suiteTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_suiteTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(m_suiteTableView, SIGNAL(doubleClicked(QModelIndex)), this,
            SLOT(showTimeHistoryTab()));
    layout->addWidget(m_suiteTableView, 0, 0);

    QGroupBox *suiteGroupBox = new QGroupBox(tr("Motions of Selected Suite"));
    suiteGroupBox->setLayout(layout);

    // Create the plots
    createTabWidget();
    m_plotsTabWidget->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Expanding);

    // Create button row
    QGridLayout *buttonRowLayout = new QGridLayout;
    buttonRowLayout->setColumnStretch(0, 1);

    m_exportPushButton = new QPushButton(QIcon(":/images/document-save-as.svg"),
                                         tr("Export Suites..."));
    connect(m_exportPushButton, SIGNAL(clicked()), SLOT(exportSuites()));
    buttonRowLayout->addWidget(m_exportPushButton, 0, 1);

    QPushButton *closePushButton = new QPushButton(tr("Close"));
    connect(closePushButton, SIGNAL(clicked()), SLOT(close()));
    buttonRowLayout->addWidget(closePushButton, 0, 2);

    // Set the layout of the entire dialog
    layout = new QGridLayout;

    layout->addWidget(suiteListGroupBox, 0, 0);
    layout->addWidget(m_plotsTabWidget, 0, 1);
    layout->addWidget(suiteGroupBox, 1, 0, 1, 2);
    layout->addLayout(buttonRowLayout, 2, 0, 1, 2);

    setLayout(layout);
}

QwtPlot *SuiteDialog::createPlot(const QString &xLabel,
                                 bool xLogAxis,
                                 const QString &yLabel,
                                 bool yLogAxis,
                                 bool trackPointer) {
    QwtPlot *plot = new QwtPlot;
    plot->setAutoReplot(true);
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showPlotContextMenu(QPoint)));

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

    // Set the axis scales
    if (xLogAxis) {
        plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
    }
    if (yLogAxis) {
        plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
    }

    if (trackPointer) {
        QwtPlotPicker *picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                                  QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly,
                                                  plot->canvas());
        picker->setStateMachine(new QwtPickerTrackerMachine());
    }

    return plot;
}

void SuiteDialog::resetCurve(QwtPlotCurve *curve) {
    curve->setPen(QPen(Qt::darkGray));
    curve->setZ(m_zOrder);
}

void SuiteDialog::highlightCurve(QwtPlotCurve *curve) {
    curve->setPen(QPen(QBrush(Qt::darkGreen), 2, Qt::SolidLine));
    curve->setZ(m_zOrder + 3);
}

void SuiteDialog::createTabWidget() {
    m_plotsTabWidget = new QTabWidget;

    // Average response spectrum plot
    if (m_motionLibrary->combineComponents()) {
        m_groupedRespSpecPlot = createPlot(tr("Period (s)"), true, tr("Spectral Accel. (g)"));
        QwtPlotPicker *picker = new QwtPlotPicker(
                QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::CrossRubberBand,
                QwtPicker::ActiveOnly, m_groupedRespSpecPlot->canvas());
        picker->setStateMachine(new QwtPickerDragPointMachine());

        connect(picker, SIGNAL(appended(QPoint)), this,
                SLOT(groupedRespSpecPointSelected(QPoint)));

        QString key = "groupedRespSpec";
        // Number of motion pairs
        for (int i = 0; i < m_motionLibrary->suiteSize(); ++i) {
            createCurve(m_groupedRespSpecPlot, key, QPen(Qt::darkGray));
        }
        // Add the median and +/- sigma curves for both the target and the suite
        key += "Stats";
        for (const QBrush &brush : {QBrush(Qt::blue), QBrush(Qt::red)}) {
            for (Qt::PenStyle penStyle : {Qt::SolidLine, Qt::DashLine, Qt::DashLine}) {
                createCurve(m_groupedRespSpecPlot, key, QPen(brush, penStyle));
            }
        }
        m_plotsTabWidget->addTab(m_groupedRespSpecPlot,
                                 tr("Grouped Response Spectra"));
    } else {
        m_groupedRespSpecPlot = 0;
    }

    // Response spectrum
    m_indivRespSpecPlot = createPlot(tr("Period (s)"), true, tr("Spectral Accel. (g)"));
    QwtPlotPicker *picker = new QwtPlotPicker(
            QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::CrossRubberBand,
            QwtPicker::ActiveOnly, m_indivRespSpecPlot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());

    connect(picker, SIGNAL(appended(QPoint)), this,
            SLOT(indivRespSpecPointSelected(QPoint)));

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


    QString key = "indivRespSpec";
    // Number of motion pairs
    for (int i = 0; i < m_motionLibrary->suites().first()->rowCount(); ++i) {
        createCurve(m_indivRespSpecPlot, key, QPen(Qt::darkGray));
    }
    // Add the median and +/- sigma curves for both the target and the suite
    key += "Stats";
    for (const QBrush &brush : {QBrush(Qt::blue), QBrush(Qt::red)}) {
        for (Qt::PenStyle penStyle : {Qt::SolidLine, Qt::DashLine, Qt::DashLine}) {
            createCurve(m_indivRespSpecPlot, key, QPen(brush, penStyle));
        }
    }
    m_plotsTabWidget->addTab(m_indivRespSpecPlot,
                             tr("Individual Response Spectra"));

    // Standard deviation
    QwtPlot *plot = createPlot(tr("Period (s)"), true,
                               tr("Std. (%1_ln)").arg(QChar(0x03C3)), false, true);
    plot->axisScaleEngine(QwtPlot::yLeft)
            ->setAttribute(QwtScaleEngine::IncludeReference, true);

    m_plotsTabWidget->addTab(plot, tr("Standard Deviation"));
    for (Qt::GlobalColor color : {Qt::blue, Qt::red}) {
        createCurve(plot, "stdev", QPen(QBrush(color), 2, Qt::SolidLine));
    }

    // Time histories
    QVBoxLayout *layout = new QVBoxLayout;
    for (const QString &yLabel : {tr("Accel. (g)"), tr("Vel. (cm/s)"), tr("Disp (cm)")}) {
        // Acceleration plot
        QString xLabel = (layout->count() == 2) ? tr("Time (sec)") : "";
        plot = createPlot(xLabel, false, yLabel, false, true);
        plot->axisScaleEngine(QwtPlot::yLeft)
                ->setAttribute(QwtScaleEngine::Symmetric, true);
        createCurve(plot, "timeSeries", QPen(Qt::blue));
        layout->addWidget(plot);
    }

    QFrame *tsFrame = new QFrame;
    // Allow the frame to shrink vertically
    tsFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    tsFrame->setLayout(layout);

    m_plotsTabWidget->addTab(tsFrame, tr("Time Series"));
}