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

#include "HelpDialog.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextBrowser>

HelpDialog::HelpDialog( QWidget *parent, Qt::WindowFlags flags )
	: QDialog(parent,flags)
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(3,1);

    QPushButton * backPushButton = new QPushButton(QIcon(":/images/go-previous.svg"), tr("Back"));
    layout->addWidget( backPushButton, 0, 0);
    
    QPushButton * forwardPushButton = new QPushButton(QIcon(":/images/go-next.svg"), tr("Forward"));
    layout->addWidget( forwardPushButton, 0, 1);

    QPushButton * homePushButton = new QPushButton(QIcon(":/images/go-home.svg"), tr("Home"));
    layout->addWidget( homePushButton, 0, 2);

    m_urlLineEdit = new QLineEdit;
    m_urlLineEdit->setReadOnly(true);
    layout->addWidget( m_urlLineEdit, 0, 3, 1, 2);


    QHBoxLayout * rowLayout = new QHBoxLayout;
    // Table of contents
    QTextBrowser * tocBrowser = new QTextBrowser;
    tocBrowser->setSource(QUrl("qrc:/doc/manual2.html"));
    tocBrowser->setOpenLinks(false);

    rowLayout->addWidget( tocBrowser );

    // Main view
    QTextBrowser * textBrowser = new QTextBrowser;
    textBrowser->setSource(QUrl("qrc:/doc/manual3.html"));
    connect( tocBrowser, SIGNAL(anchorClicked(QUrl)), textBrowser, SLOT(setSource(QUrl)));
    connect( backPushButton, SIGNAL(clicked()), textBrowser, SLOT(backward()));
    connect( textBrowser, SIGNAL(backwardAvailable(bool)), backPushButton, SLOT(setEnabled(bool)));
    connect( forwardPushButton, SIGNAL(clicked()), textBrowser, SLOT(forward()));
    connect( textBrowser, SIGNAL(forwardAvailable(bool)), forwardPushButton, SLOT(setEnabled(bool)));
    connect( homePushButton, SIGNAL(clicked()), textBrowser, SLOT(home()));
    connect( textBrowser, SIGNAL(sourceChanged(QUrl)), SLOT(updateUrl(QUrl)));

    rowLayout->addWidget( textBrowser, 1 );

    layout->addLayout( rowLayout, 1, 0, 1, 5);

    QPushButton * closePushButton = new QPushButton(tr("Close"));
    connect( closePushButton, SIGNAL(clicked()), SLOT(close()));
    layout->addWidget( closePushButton, 2, 4);

    setLayout(layout);
}
        
void HelpDialog::updateUrl(const QUrl & url)
{
    m_urlLineEdit->setText(url.toString());
}
