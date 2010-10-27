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


#ifndef EXPORT_DIALOG_H_
#define EXPORT_DIALOG_H_

#include <QDialog>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>

#include "MotionSuite.h"

class ExportDialog : public QDialog
{
    Q_OBJECT

    public:
        ExportDialog( const QList<MotionSuite*> & suites, const QString & motionPath, QWidget * parent = 0, Qt::WindowFlags f = 0 );

    private slots:
        void selectDestination();
        void tryAccept();

    private:
        QRadioButton * m_noRadioButton;
        QRadioButton * m_csvRadioButton;
        QRadioButton * m_strataRadioButton;
        QRadioButton * m_shake2kRadioButton;

        QCheckBox * m_summaryCheckBox;

        QLineEdit * m_destinationLineEdit;

        QLineEdit * m_prefixLineEdit;

        const QList<MotionSuite*> & m_suites;
        QString m_motionPath;
};
#endif
