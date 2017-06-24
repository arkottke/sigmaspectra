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

#ifndef SUITEDIALOG_H_
#define SUITEDIALOG_H_

#include "MotionLibrary.h"
#include "MotionSuite.h"

#include <QDialog>
#include <QList>
#include <QTabWidget>
#include <QTableView>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class SuiteDialog : public QDialog {
Q_OBJECT

public:
    SuiteDialog(MotionLibrary *motionLibrary, QWidget *parent = 0, Qt::WindowFlags f = 0);

    ~SuiteDialog();

public slots:

    //! Window to configure plot
    void configurePlot();

private slots:

    void suiteSelected();

    void motionSelected();

    void showTimeHistoryTab();

    void exportSuites();

    void groupedRespSpecPointSelected(const QPoint &point);

    void indivRespSpecPointSelected(const QPoint &point);

    //! Show the context menu
    void showPlotContextMenu(const QPoint &point);

    void copyPlot();

signals:

protected:
    //! Create a
    //! Plot the response spectrum of the current selected suite
    void plotSelectedSuite();

    //! Create the page
    void createPage();

    //! Create the tab widget for displaying plots
    void createTabWidget();

    //! Create a curve and add it to the collection
    QwtPlotCurve *createCurve(QwtPlot *plot, QString key, QPen pen, double zOrder = 20);

    //! Helper function to create a plot and connect it with the widget
    QwtPlot *createPlot(const QString &xLabel = "",
                        bool xLogAxis = false,
                        const QString &yLabel = "",
                        bool yLogAxis = false,
                        bool trackPointer = false
    );

    //! Reset a curve to the default settings
    void resetCurve(QwtPlotCurve *curve);

    //! Highlight a curve
    void highlightCurve(QwtPlotCurve *curve);


    //! The plot of the response spectrum
    QwtPlot *m_groupedRespSpecPlot;

    //! The plot of the response spectrum
    QwtPlot *m_indivRespSpecPlot;

    //! Table view of all of the suites
    QTableView *m_suiteListTableView;

    //! Table view of the currently selected suite
    QTableView *m_suiteTableView;

    //! Push button used to export the selected suite
    QPushButton *m_exportPushButton;

    //! Tabs for different plots
    QTabWidget *m_plotsTabWidget;

    //! Motion library containing the target response spectrum and suites
    MotionLibrary *m_motionLibrary;

    //! Currently selected suite
    MotionSuite *m_selectedSuite;

    //! All curves used for displaying the results
    QMap<QString, QwtPlotCurve *> m_curves;

    //! Curves selected in the plots
    QList<QwtPlotCurve *> m_selectedCurves;

    //! Default zOrder for the curves
    int m_zOrder;

    //! Context menu for the plots
    QMenu *m_plotContextMenu;

    //! Current plot -- used for the context menus
    QwtPlot *m_currentPlot;
};

#endif
