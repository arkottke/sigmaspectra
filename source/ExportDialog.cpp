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

#include "ExportDialog.h"

#include <QCompleter>
#include <QDialogButtonBox>
#include <QDirModel>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

#include <QDebug>

ExportDialog::ExportDialog(const QList<MotionSuite *> &suites, const QString &motionPath, QWidget *parent,
                           Qt::WindowFlags f)
        : QDialog(parent, f), m_suites(suites), m_motionPath(motionPath) {
    QSettings settings;

    // File type
    QGridLayout *layout = new QGridLayout;
    layout->setColumnMinimumWidth(0, 20);

    m_noRadioButton = new QRadioButton(tr("No Output"));
    layout->addWidget(m_noRadioButton, 0, 0, 1, 2);

    m_csvRadioButton = new QRadioButton(tr("Comma separated values (CSV)"));
    layout->addWidget(m_csvRadioButton, 1, 0, 1, 2);
    layout->addWidget(new QLabel(tr("-- suitable for use with spreadsheets such as Excel")), 2, 1);

    m_strataRadioButton = new QRadioButton(tr("Strata suite file"));
    layout->addWidget(m_strataRadioButton, 3, 0, 1, 2);
    layout->addWidget(new QLabel(tr("-- suitable for use with Strata")), 4, 1);

    m_shake2kRadioButton = new QRadioButton(tr("SHAKE2000 suite file"));
    layout->addWidget(m_shake2kRadioButton, 5, 0, 1, 2);
    layout->addWidget(new QLabel(tr("-- suitable for use with SHAKE2000")), 6, 1);

    // Load initial file type
    switch (settings.value("exportDialog/outputType", MotionSuite::CSVOutput).toInt()) {
        case MotionSuite::NoOutput:
            m_noRadioButton->setChecked(true);
            break;
        case MotionSuite::StrataOutput:
            m_strataRadioButton->setChecked(true);
            break;
        case MotionSuite::SHAKE2000Output:
            m_shake2kRadioButton->setChecked(true);
            break;
        case MotionSuite::CSVOutput:
        default:
            m_csvRadioButton->setChecked(true);
            break;
    }

    QGroupBox *groupBox = new QGroupBox(tr("Output format of selected suites"));
    groupBox->setLayout(layout);

    layout = new QGridLayout;
    layout->addWidget(groupBox, 0, 0, 1, 2);

    // Prefix
    m_prefixLineEdit = new QLineEdit;
    m_prefixLineEdit->setText(settings.value("exportDialog/prefix", "suite").toString());
    connect(m_noRadioButton, SIGNAL(toggled(bool)), m_prefixLineEdit, SLOT(setDisabled(bool)));
    connect(m_shake2kRadioButton, SIGNAL(toggled(bool)), m_prefixLineEdit, SLOT(setDisabled(bool)));

    layout->addWidget(new QLabel(tr("Prefix:")), 1, 0);
    layout->addWidget(m_prefixLineEdit, 1, 1);

    // Destination folder
    QPushButton *pushButton = new QPushButton(tr("Path..."));
    pushButton->setAutoDefault(false);
    connect(pushButton, SIGNAL(clicked()), SLOT(selectDestination()));
    layout->addWidget(pushButton, 2, 0);

    m_destinationLineEdit = new QLineEdit;
    m_destinationLineEdit->setText(settings.value("exportDialog/destination", QDir::currentPath()).toString());
    // QCompleter * completer = new QCompleter(this);
    // completer->setModel(new QDirModel(completer));
    // m_destinationLineEdit->setCompleter(completer);
    layout->addWidget(m_destinationLineEdit, 2, 1);

    // Include summary
    m_summaryCheckBox = new QCheckBox(tr("Include a summary of all generated suites"));
    m_summaryCheckBox->setChecked(settings.value("exportDialog/summary", false).toBool());

    layout->addWidget(m_summaryCheckBox, 3, 0, 1, 2);

    // Dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), SLOT(tryAccept()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
    layout->addWidget(buttonBox, 4, 0, 1, 2);

    setLayout(layout);
}

void ExportDialog::selectDestination() {
    QSettings settings;

    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    dialog.setWindowTitle(tr("Select destination directory..."));
    dialog.setViewMode(QFileDialog::Detail);

    if (settings.contains("exportDialog/pathDialog"))
        dialog.restoreState(settings.value("exportDialog/pathDialog").toByteArray());

    dialog.setDirectory(m_destinationLineEdit->text());

    if (dialog.exec()) {
        // Save the state
        settings.setValue("exportDialog/pathDialog", dialog.saveState());

        m_destinationLineEdit->setText(QDir::toNativeSeparators(dialog.selectedFiles().first()));
    }
}

void ExportDialog::tryAccept() {
    MotionSuite::OutputType type = MotionSuite::NoOutput;

    if (m_csvRadioButton->isChecked()) {
        type = MotionSuite::CSVOutput;
    } else if (m_strataRadioButton->isChecked()) {
        type = MotionSuite::StrataOutput;
    } else if (m_shake2kRadioButton->isChecked()) {
        type = MotionSuite::SHAKE2000Output;
    }

    // Save old values in the settings
    QSettings settings;
    settings.setValue("exportDialog/outputType", int(type));
    settings.setValue("exportDialog/summary", m_summaryCheckBox->isChecked());
    settings.setValue("exportDialog/destination", m_destinationLineEdit->text());
    settings.setValue("exportDialog/prefix", m_prefixLineEdit->text());

    // Make the directory if it doesn't exist
    QDir destDir(m_destinationLineEdit->text());

    if (!destDir.exists()) {
        // Prompt for the creation
        int ret = QMessageBox::question(this, tr("Strata"),
                                        QString(tr("The directory '%1' does not exist.\n Do you want to "
                                                           "create this directory?"))
                                                .arg(destDir.absolutePath()),
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if (ret == QMessageBox::No)
            return;
        else
            // Create the directory
            destDir.mkpath(".");
    }

    QString baseName = m_destinationLineEdit->text() + QDir::separator() + m_prefixLineEdit->text();

    switch (type) {
        case MotionSuite::NoOutput:
        case MotionSuite::SummaryOutput:
            // Do nothing
            break;
        case MotionSuite::CSVOutput:
            for (int i = 0; i < m_suites.size(); ++i) {
                if (!m_suites.at(i)->enabled()) {
                    continue;
                }

                QFile file(QString("%1-%2.csv").arg(baseName).arg(i + 1));

                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) { 
                    qCritical() << "Unable to open file:" << file.fileName();
                }

                QTextStream out(&file);
                m_suites.at(i)->toText(out, MotionSuite::CSVOutput);
            }
            break;
        case MotionSuite::StrataOutput:
            for (int i = 0; i < m_suites.size(); ++i) {
                if (!m_suites.at(i)->enabled())
                    continue;

                QFile file(QString("%1-%2.csv").arg(baseName).arg(i + 1));

                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
                    qCritical() << "Unable to open file:" << file.fileName();
                }

                QTextStream out(&file);
                m_suites.at(i)->toText(out, MotionSuite::StrataOutput);
            }
            break;
        case MotionSuite::SHAKE2000Output: {
            QFile file(QDir::currentPath() + QDir::separator() + "SuiteLog.txt");

            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                qCritical() << "Unable to open file:" << file.fileName();
            }

            QTextStream out(&file);

            out << m_motionPath << endl
                << endl;

            for (int i = 0; i < m_suites.size(); ++i) {
                if (!m_suites.at(i)->enabled())
                    continue;

                out << QString("[ %1 of %2 ]\n").arg(i + 1).arg(m_suites.size());
                m_suites.at(i)->toText(out, MotionSuite::SHAKE2000Output);
                out << endl;
            }

            break;
        }
    }

    if (m_summaryCheckBox->isChecked()) {
        QString destinationDir = (type == MotionSuite::SHAKE2000Output) ? QDir::currentPath()
                                                                        : m_destinationLineEdit->text();

        QFile file(destinationDir + QDir::separator() + "summary.csv");

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qCritical() << "Unable to open file:" << file.fileName();
        }

        QTextStream out(&file);

        for (int i = 0; i < m_suites.size(); ++i) {
            out << QString("[ %1 of %2 ]\n").arg(i + 1).arg(m_suites.size());
            m_suites.at(i)->toText(out, MotionSuite::SummaryOutput);
            out << endl
                << endl;
        }
    }

    accept();
}
