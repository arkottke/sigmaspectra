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


#include "InputTableModel.h"

InputTableModel::InputTableModel( MotionLibrary* motionLibrary, QObject* parent )
    : QAbstractTableModel(parent), m_motionLibrary( motionLibrary )
{
    m_nCols = 3;
}

int InputTableModel::rowCount(const QModelIndex& /*index*/) const
{
    return m_motionLibrary->inputPeriod().size();
}

int InputTableModel::columnCount(const QModelIndex& /*index*/) const
{
    return m_nCols;
}

QVariant InputTableModel::data(const QModelIndex& index, int role) const
{
    if( index.parent()!=QModelIndex()) 
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole) {
        switch( index.column() ) {
        case PERIOD_COLUMN:
            return QString::number(m_motionLibrary->inputPeriod().at(index.row()));
        case SPEC_ACCEL_COLUMN:
            return QString::number(m_motionLibrary->inputSa().at(index.row()));
        case LN_STDEV_COLUMN:
            return QString::number(m_motionLibrary->inputLnStd().at(index.row()));
        default:
            return QVariant();
        }
    } else {
        return QVariant();
    }
}

bool InputTableModel::setData( const QModelIndex& index, const QVariant& value, int role)
{
    if(index.parent()!=QModelIndex())
        return false;

    // Only allow postive numbers to be entered
    if (value.toDouble() < 0) {
        qCritical("Only positive values can be entered");
        return false;
    }

    if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole) {
        switch(index.column()) {
        case PERIOD_COLUMN:
            if (value.toDouble() <= 0) {
                qCritical("Period must be greater than 0.");
                return false;
            }
            m_motionLibrary->inputPeriod()[index.row()] =  value.toDouble();
            break;
        case SPEC_ACCEL_COLUMN:
            m_motionLibrary->inputSa()[index.row()] =  value.toDouble();
            break;
        case LN_STDEV_COLUMN:
            m_motionLibrary->inputLnStd()[index.row()] =  value.toDouble();
            break;
        default:
            return false;
        }
        emit dataChanged(index, index);
        return true;
    } else {
        return false;
    }
}

Qt::ItemFlags InputTableModel::flags( const QModelIndex& index ) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

QVariant InputTableModel::headerData( int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
        return QVariant();

    switch (orientation) {
    case Qt::Horizontal:
        switch (section) {
        case PERIOD_COLUMN:
            return tr("Period (s)");
        case SPEC_ACCEL_COLUMN:
            return tr("Spec. Accel. (g)");
        case LN_STDEV_COLUMN:
            return tr("Ln Stdev.");
        default:
            return QVariant();
        }        
    case Qt::Vertical:
        return QVariant(section+1);

    default:
        return QVariant();
    }
}

bool InputTableModel::insertRows ( int row, int count, const QModelIndex &parent )
{
    emit beginInsertRows( parent, row, row+count-1  );

    m_motionLibrary->inputPeriod().insert( row, count, 0.0 );
    m_motionLibrary->inputSa().insert( row, count, 0.0 );
    m_motionLibrary->inputLnStd().insert( row, count, 0.0 );

    emit endInsertRows();

    return true;
}

bool InputTableModel::removeRows ( int row, int count, const QModelIndex &parent )
{
    emit beginRemoveRows( parent, row, row+count-1);

    m_motionLibrary->inputPeriod().remove( row, count );
    m_motionLibrary->inputSa().remove( row, count );
    m_motionLibrary->inputLnStd().remove( row, count );

    emit endRemoveRows();

    return true;
}
