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

#ifndef MY_TABLE_VIEW_H_
#define MY_TABLE_VIEW_H_

#include <QContextMenuEvent>
#include <QMenu>
#include <QTableView>

class MyTableView : public QTableView {
Q_OBJECT

public:
    MyTableView(QWidget *parent = 0);

signals:

    void dataPasted();

public slots:

    void copy();

    void paste();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    QMenu *m_contextMenu;
};

#endif
