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


#ifndef TARGETTABLEMODEL_H_
#define TARGETTABLEMODEL_H_

#include <QAbstractTableModel>
#include <QVariant>
#include <QModelIndex>

#include "MotionLibrary.h"

class InputTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    InputTableModel( MotionLibrary* motionLibrary, QObject* parent = 0 );

    int rowCount ( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount ( const QModelIndex &parent = QModelIndex() ) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData( const QModelIndex& index, const QVariant & value, int role = Qt::EditRole );

    Qt::ItemFlags flags( const QModelIndex& index ) const;
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    bool insertRows ( int row, int count, const QModelIndex &parent = QModelIndex() );
    bool removeRows ( int row, int count, const QModelIndex &parent = QModelIndex() );

private:
    enum Columns {
        PERIOD_COLUMN,
        SPEC_ACCEL_COLUMN,
        LN_STDEV_COLUMN
    };

    int m_nCols;

    MotionLibrary *m_motionLibrary;
};
#endif
