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


#include "MainWindow.h"
#include <QApplication>
#include <QMessageBox>

void myMessageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s\n", msg);
        /*
                        if ( logBox == 0 )
                                logBox = new QTextEdit;

                        logBox->append( msg );
                        */
        break;
    case QtWarningMsg:
        QMessageBox::warning(0, "SigmaSpectra", msg);
        break;
    case QtCriticalMsg:
        QMessageBox::critical(0, "SigmaSpectra", msg);
        break;
    case QtFatalMsg:
        QMessageBox::critical(0, "SigmaSpectra", msg);
        //abort();
    }
}

int main( int argc, char* argv[] )
{
    //qInstallMsgHandler(myMessageOutput);
    
    QCoreApplication::setOrganizationName("Albert Kottke");
    QCoreApplication::setOrganizationDomain("accipter.org");
    QCoreApplication::setApplicationName("SigmaSpectra");

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/application-icon.svg"));

    MainWindow *window = new MainWindow;
    window->show();
    return app.exec();
}

