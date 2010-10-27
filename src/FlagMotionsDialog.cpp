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

#include "FlagMotionsDialog.h"

#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QSplitter>
#include <QTableView>
#include <QVBoxLayout>

#include <QDebug>

#include <qwt_plot.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>

#include "Motion.h"
#include "MotionPair.h"
#include "StringListDelegate.h"
#include "SuiteDialog.h"

FlagMotionsModel::FlagMotionsModel(QList<AbstractMotion*> & motions, QObject * parent)
       : QAbstractTableModel(parent), m_motions(motions)
{
}

int FlagMotionsModel::rowCount(const QModelIndex &parent) const
{
    return m_motions.size();
}

int FlagMotionsModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant FlagMotionsModel::data(const QModelIndex& index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    // Icon of the row
    if (index.column() == 0 &&
        role == Qt::DecorationRole) {
        switch (m_motions.at(index.row())->flag()) {
            case AbstractMotion::Required:
                return QIcon(":/images/flags-required.png");
            case AbstractMotion::Requested:
                return QIcon(":/images/flags-requested.png");
            case AbstractMotion::Unmarked:
                return QIcon(":/images/flags-unmarked.png");
            case AbstractMotion::Disabled:
                return QIcon(":/images/flags-disabled.png");
            }
    }

    // String data
    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        switch (index.column()) {
        case 0:
            if (role == Qt::EditRole) {
                QMap<QString, QVariant> map;
                map.insert("list", QVariant(AbstractMotion::flags()));
                map.insert("index", (int)m_motions.at(index.row())->flag());
                return map;
            } else {
                return AbstractMotion::flags().at((int)m_motions.at(index.row())->flag());
            }
        case 1:
            // Name
            return m_motions.at(index.row())->name();
        }
    }

    return QVariant();
}

bool FlagMotionsModel::setData(const QModelIndex& index, const QVariant & value, int role)
{
    if(index.parent()!=QModelIndex())
        return false;

    if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole) {
        switch (index.column())
        {
        case 0:
            // Flags
            m_motions[index.row()]->setFlag((AbstractMotion::Flag)value.toInt());
            break;
        default:
            return false;
        }
    }

    dataChanged(index, index);
    return true;
}

Qt::ItemFlags FlagMotionsModel::flags(const QModelIndex& index) const
{
    if (index.column() == 1 ) {
        // Name column -- not editable
        return QAbstractTableModel::flags(index);
    } else {
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    }
}

QVariant FlagMotionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
        return QVariant();

    switch( orientation ) {
    case Qt::Horizontal:
        switch (section) {
        case 0:
            // Name
            return tr("Flag");
        case 1:
            // Scale
            return tr("Name");

        default:
            return QVariant();
        }
    case Qt::Vertical:
        return QVariant(section+1);
                default:
        return QVariant();
    }
}

FlagMotionsDialog::FlagMotionsDialog(QList<AbstractMotion*> & motions, QWidget * parent, Qt::WindowFlags f)
        : QDialog(parent, f), m_motions(motions)
{
    QSplitter * splitter = new QSplitter;

    // Table View
    QTableView * tableView = new QTableView;
    tableView->setModel(new FlagMotionsModel(m_motions));
    tableView->resizeRowsToContents();
    tableView->setItemDelegateForColumn(0, new StringListDelegate);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(selectMotion(QModelIndex, QModelIndex)));

    splitter->addWidget(tableView);

    // Plot of time series
    QVBoxLayout * tsLayout = new QVBoxLayout;
    QList<QwtPlot*> plots;

    // Acceleration plot
    QwtPlot* plot = new QwtPlot;
    SuiteDialog::configurePlot(plot, "", tr("Accel. (g)"));
    tsLayout->addWidget(plot);

    plots << plot;

    // Velocity plot
    plot = new QwtPlot;
    SuiteDialog::configurePlot(plot, "", tr("Vel. (cm/s)"));
    tsLayout->addWidget(plot);

    plots << plot;

    // Displacment plot
    plot = new QwtPlot;
    SuiteDialog::configurePlot(plot, tr("Time (s)"), tr("Disp. (cm)"));
    tsLayout->addWidget(plot);

    plots << plot;

    // Add curves to each of the plots
    m_count = dynamic_cast<Motion*>(m_motions.first()) ? 1 : 2;
    foreach (QwtPlot * plot, plots) {
        // Set properties shared acrossed the plots
        plot->setAutoReplot(true);
        plot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Symmetric, true);

        // Plots become too large vertically if not ignored
        plot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

        QwtPlotPicker * picker = new QwtPlotPicker(
                   QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection,
                   QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, plot->canvas());

        Q_UNUSED(picker);
        for (int i = 0; i < m_count; ++i) {
            m_tsCurves << new QwtPlotCurve;
            m_tsCurves.last()->attach(plot);
            m_tsCurves.last()->setPen(
                    i == 0 ? QPen(Qt::blue) : QPen(Qt::green));
        }
    }
    QFrame * tsFrame = new QFrame;
    tsFrame->setLayout(tsLayout);

    splitter->addWidget(tsFrame);
    splitter->setStretchFactor(1,1);

    // Create layout and add splitter
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0, 1);
    layout->addWidget(splitter, 0, 0, 1, 2);

    // Button row
    QPushButton * closeButton = new QPushButton(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), SLOT(accept()));
    layout->addWidget(closeButton, 1, 1);

    setLayout(layout);

    resize(800, 600);

    // Select the first row -- need to select the data before creating the layout otherwise the size is updated with the new data
    tableView->selectRow(0);
}

void FlagMotionsDialog::selectMotion(const QModelIndex & current, const QModelIndex & previous)
{
    Q_UNUSED(previous);

    for (int j = 0; j < m_count; ++j) {
        const Motion * m = 0;
        if (m_count == 1)
            // Single motions
            m = dynamic_cast<Motion*>(m_motions.at(current.row()));
        else {
            const MotionPair * mp = dynamic_cast<MotionPair*>(m_motions.at(current.row()));
            m = (j == 0) ? mp->motionA() : mp->motionB();
        }

        // Set the data
        m_tsCurves[0 + j]->setData(m->time().data(), m->acc().data(), m->time().size());
        m_tsCurves[m_count*1 + j]->setData(m->time().data(), m->vel().data(), m->time().size());
        m_tsCurves[m_count*2 + j]->setData(m->time().data(), m->disp().data(), m->time().size());
    }
}

