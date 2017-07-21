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

#include "FlagMotionsDialog.h"

#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QSplitter>
#include <QTableView>

#include <qwt_picker_machine.h>
#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_engine.h>

#include "Motion.h"
#include "MotionPair.h"
#include "StringListDelegate.h"
#include "SuiteDialog.h"

FlagMotionsModel::FlagMotionsModel(QList<AbstractMotion *> &motions, QObject *parent)
        : QAbstractTableModel(parent), m_motions(motions) {
}

int FlagMotionsModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_motions.size();
}

int FlagMotionsModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 2;
}

QVariant FlagMotionsModel::data(const QModelIndex &index, int role) const {
    if (index.parent() != QModelIndex()) {
        return QVariant();
    }

    // Icon of the row
    if (index.column() == 0 && role == Qt::DecorationRole) {
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
                    map.insert("index", (int) m_motions.at(index.row())->flag());
                    return map;
                } else {
                    return AbstractMotion::flags().at((int) m_motions.at(index.row())->flag());
                }
            case 1:
                // Name
                return m_motions.at(index.row())->name();
        }
    }

    return QVariant();
}

bool FlagMotionsModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.parent() != QModelIndex()) {
        return false;
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        switch (index.column()) {
            case 0:
                // Flags
                m_motions[index.row()]->setFlag((AbstractMotion::Flag) value.toInt());
                break;
            default:
                return false;
        }
    }

    dataChanged(index, index);
    return true;
}

Qt::ItemFlags FlagMotionsModel::flags(const QModelIndex &index) const {
    if (index.column() == 1) {
        // Name column -- not editable
        return QAbstractTableModel::flags(index);
    } else {
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    }
}

QVariant FlagMotionsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole) {
        return QVariant();
    }

    switch (orientation) {
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
            return QVariant(section + 1);
        default:
            return QVariant();
    }
}

FlagMotionsDialog::FlagMotionsDialog(QList<AbstractMotion *> &motions, QWidget *parent, Qt::WindowFlags f)
        : QDialog(parent, f), m_motions(motions) {
    m_groupedMotions = bool(dynamic_cast<MotionPair *>(m_motions.first()));
    m_curves.resize(m_groupedMotions ? 6 : 3);

    QSplitter *splitter = new QSplitter;

    // Table View
    QTableView *tableView = new QTableView;
    tableView->setModel(new FlagMotionsModel(m_motions));
    tableView->resizeColumnsToContents();
    tableView->resizeRowsToContents();
    tableView->setItemDelegateForColumn(0, new StringListDelegate);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(selectMotion(QModelIndex, QModelIndex)));

    splitter->addWidget(tableView);

    // Plot of time series FIXME
    QVBoxLayout *tsLayout = new QVBoxLayout;
    QList<QString> yLabels = {tr("Accel. (g)"), tr("Vel. (cm/s)"), tr("Disp (cm)")};
    QList<Qt::GlobalColor> colors = {Qt::blue};
    if (m_groupedMotions) {
        colors << Qt::red;
    }

    for (int i = 0; i < yLabels.size(); ++i) {
        QwtPlot *plot = new QwtPlot;
        plot->setAutoReplot(true);
        plot->axisScaleEngine(QwtPlot::yLeft)
                ->setAttribute(QwtScaleEngine::Symmetric, true);
        QwtPlotPicker *picker = new QwtPlotPicker(
                QwtPlot::xBottom, QwtPlot::yLeft,
                QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly,
                plot->canvas());
        picker->setStateMachine(new QwtPickerTrackerMachine());

        // Label the axes
        QFont font = QApplication::font();
        plot->setAxisFont(QwtPlot::xBottom, font);
        plot->setAxisFont(QwtPlot::yLeft, font);

        font.setBold(true);
        QwtText text;
        text.setFont(font);

        // XLabel
        if (tsLayout->count() == 2) {
            text.setText(tr("Time (sec)"));
            plot->setAxisTitle(QwtPlot::xBottom, text);
        }
        text.setText(yLabels.at(i));
        plot->setAxisTitle(QwtPlot::yLeft, text);

        tsLayout->addWidget(plot);
        for (int j = 0; j < colors.size(); ++j) {
            QwtPlotCurve *curve = new QwtPlotCurve;
            curve->setPen(QPen(colors.at(j)));
            curve->setRenderHint(QwtPlotItem::RenderAntialiased);
            curve->attach(plot);
            // One set of colors and then the next set of three
            m_curves[i + yLabels.size() * j] = curve;
        }
    }

    QFrame *tsFrame = new QFrame;
    tsFrame->setLayout(tsLayout);

    splitter->addWidget(tsFrame);
    splitter->setStretchFactor(1, 1);

    // Create layout and add splitter
    QGridLayout *layout = new QGridLayout;
    layout->setColumnStretch(0, 1);
    layout->addWidget(splitter, 0, 0, 1, 2);

    // Button row
    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), SLOT(accept()));
    layout->addWidget(closeButton, 1, 1);

    setLayout(layout);

    resize(800, 600);

    // Select the first row -- need to select the data before creating the layout otherwise the size is updated with the new data
    tableView->selectRow(0);
}

void FlagMotionsDialog::selectMotion(const QModelIndex &current, const QModelIndex &previous) {
    Q_UNUSED(previous);
    AbstractMotion *absMotion = m_motions.at(current.row());
    int i = 0;
    if (m_groupedMotions) {
        MotionPair *mp = dynamic_cast<MotionPair *>(absMotion);
        for (const Motion *m : {mp->motionA(), mp->motionB()}) {
            for (const QVector<double> &values : {m->acc(), m->vel(), m->disp()}) {
                qDebug() << i << m_curves[i];
                m_curves[i]->setSamples(m->time(), values);
                ++i;
            }
        }
    } else {
        Motion *m = dynamic_cast<Motion *>(absMotion);
        for (const QVector<double> &values : {m->acc(), m->vel(), m->disp()}) {
            m_curves[i]->setSamples(m->time(), values);
            ++i;
        }
    }
}
