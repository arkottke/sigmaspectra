////////////////////////////////////////////////////////////////////////////////////
// This file is part of SigmaSpectra.
//
// SigmaSpectra is free software: you can redistribute it and/or modify it under
// the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// SigmaSpectra is distributed in the hope that it will be useful, but WITHOUT
// ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A
// PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// SigmaSpectra.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2008-2017 Albert Kottke
////////////////////////////////////////////////////////////////////////////////////

#include "MainWindow.h"
#include "defines.h"

#include <QApplication>
#include <QMessageBox>

void debugHandler(QtMsgType type, const QMessageLogContext &context,
        const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(),
                    context.file, context.line, context.function);
            break;
        case QtInfoMsg:
            fprintf(stdout, "Info: %s\n", localMsg.constData());
            fflush(stdout);
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(),
                    context.file, context.line, context.function);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(),
                    context.file, context.line, context.function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(),
                    context.file, context.line, context.function);
            abort();
    }
}

void releaseHandler(QtMsgType type, const QMessageLogContext &context,
        const QString &msg) {

    QWidget *widget = QApplication::activeWindow();

    switch (type) {
        case QtDebugMsg:
            QMessageBox::information(
                    widget,
                    QString("%1 - %2").arg(PROJECT_LONGNAME).arg("Debug"),
                    msg);
            break;
        case QtInfoMsg:
            QMessageBox::information(
                    widget,
                    QString("%1 - %2").arg(PROJECT_LONGNAME).arg("Information"),
                    msg);
            break;
        case QtWarningMsg:
            QMessageBox::warning(
                    widget,
                    QString("%1 - %2").arg(PROJECT_LONGNAME).arg("Warning"),
                    msg);
            break;
        case QtCriticalMsg:
            QMessageBox::critical(
                    widget,
                    QString("%1 - %2").arg(PROJECT_LONGNAME).arg("Critical"),
                    msg);
            break;
        case QtFatalMsg:
            QMessageBox::critical(
                    widget,
                    QString("%1 - %2").arg(PROJECT_LONGNAME).arg("Fatal"),
                    msg);
            abort();
    }
}

int main(int argc, char *argv[]) {
#ifdef Debug
    qInstallMessageHandler(debugHandler);
#else
    qInstallMessageHandler(releaseHandler);
#endif

    QCoreApplication::setOrganizationName("ARKottke");
    QCoreApplication::setApplicationName(PROJECT_LONGNAME);
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/application-icon.svg"));

    MainWindow *window = new MainWindow;
    window->show();
    return app.exec();
}
