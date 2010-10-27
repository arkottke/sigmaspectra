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


#ifndef SUITEDIALOG_H_
#define SUITEDIALOG_H_

#include "MotionLibrary.h"
#include "MotionSuite.h"

#include <QDialog>
#include <QList>
#include <QTableView>
#include <QTabWidget>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class SuiteDialog : public QDialog
{
    Q_OBJECT

public:
    SuiteDialog( MotionLibrary * motionLibrary, QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~SuiteDialog();

    //! Configure the fonts of the plot
    static void configurePlot(QwtPlot* plot, const QString& xLabel, const QString& yLabel);

public slots:

private slots:
    void suiteSelected();
    void motionSelected();
    void showTimeHistoryTab();

    void exportSuites();

    void groupedRespSpecPointSelected(const QPoint & point);
    void indivRespSpecPointSelected(const QPoint & point);

    //! Show the context menu
    void showPlotContextMenu(const QPoint & point);

    void copyPlot();
    void configurePlot();

signals:

protected:
    //! Create a
    //! Plot the response spectrum of the current selected suite
    void plotSelectedSuite();

    //! Create the page
    void createPage();

    //! Create the tab widget for displaying plots
    void createTabWidget();

    //! The plot of the response spectrum
    QwtPlot * m_groupedRespSpecPlot;

    //! The plot of the response spectrum
    QwtPlot * m_indivRespSpecPlot;

    //! Table view of all of the suites
    QTableView * m_suiteListTableView;

    //! Table view of the currently selected suite
    QTableView * m_suiteTableView;

    //! Push button used to export the selected suite
    QPushButton * m_exportPushButton;

    //! Tabs for different plots
    QTabWidget * m_plotsTabWidget;

    //! Motion library containing the target response spectrum and suites
    MotionLibrary * m_motionLibrary;

    //! Currently selected suite
    MotionSuite * m_selectedSuite;

    //! Currently selected motion index
    int m_selectedMotionIndex;

    //! List of the group response spectrum curves
    QList<QwtPlotCurve*> m_groupedRespSpecCurves;

    //! List of the indiviual response spectrum curves
    QList<QwtPlotCurve*> m_indivRespSpecCurves;

    //! List of the standard deviation curves
    QList<QwtPlotCurve*> m_stdCurves;

    //! List of the time series curves
    QList<QwtPlotCurve*> m_tsCurves;

    //! Default zOrder for the curves
    int m_zOrder;

    //! Count of motions
    int m_motionCount;

    //! Context menu for the plots
    QMenu * m_plotContextMenu;

    //! Current plot -- used for the context menus
    QwtPlot * m_currentPlot;
};
#endif
