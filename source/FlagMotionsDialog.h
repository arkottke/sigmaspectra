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

#ifndef FLAGMOTIONSDIALOG_H
#define FLAGMOTIONSDIALOG_H

#include <QAbstractTableModel>
#include <QDialog>
#include <QList>

#include <qwt_plot_curve.h>

#include "AbstractMotion.h"

class FlagMotionsModel : public QAbstractTableModel {
Q_OBJECT

public:
    FlagMotionsModel(QList<AbstractMotion *> &motions, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
    QList<AbstractMotion *> &m_motions;
};

class FlagMotionsDialog : public QDialog {
Q_OBJECT

public:
    FlagMotionsDialog(QList<AbstractMotion *> &motions, QWidget *parent = 0, Qt::WindowFlags f = 0);

protected slots:

    void selectMotion(const QModelIndex &current, const QModelIndex &previous);

protected:
    //! List of motions
    QList<AbstractMotion *> &m_motions;

    //! If the motions are grouped
    bool m_groupedMotions;

    //! Curves for each of the plots
    QVector<QwtPlotCurve *> m_curves;
};

#endif // FLAGMOTIONSDIALOG_H
