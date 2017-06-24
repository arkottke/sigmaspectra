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

#ifndef HELPDIALOG_H_
#define HELPDIALOG_H_

#include <QDialog>
#include <QLineEdit>
#include <QUrl>

class HelpDialog : public QDialog {
Q_OBJECT

public:
    HelpDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

public slots:

signals:

protected slots:

    void updateUrl(const QUrl &url);

private:
    QLineEdit *m_urlLineEdit;
};

#endif
