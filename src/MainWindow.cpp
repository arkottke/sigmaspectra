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
#include "FlagMotionsDialog.h"
#include "InputTableModel.h"
#include "SuiteDialog.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDomDocument>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QToolBar>
#include <QVBoxLayout>
#include <QtAlgorithms>

#include <QFileDialog>
#include <QDebug>

#include <limits>
#include <cfloat>

MainWindow::MainWindow(QMainWindow * parent) 
	: QMainWindow(parent)
{
    m_settings = new QSettings;
    m_motionLibrary = new MotionLibrary;
    connect(this, SIGNAL(disabledCountChanged(int)),
            m_motionLibrary, SLOT(setDisabledCount(int)));
    connect(m_motionLibrary, SIGNAL(motionCountChanged(int)),
            this, SLOT(updateMotionCount(int)));
   
    m_helpDialog = new HelpDialog(this);
    m_helpDialog->setWindowModality(Qt::NonModal);
    
    // Setup up the mainwindow
    createActions();
    //createToolBar();
    createMenus();
    createPage();

    // Load the values from the model
    loadValues();
}

MainWindow::~MainWindow()
{
    delete m_settings;
    delete m_motionLibrary;
    delete m_helpDialog;
}

void MainWindow::copy()
{
    QMetaObject::invokeMethod(QApplication::focusWidget(), "copy");
}

void MainWindow::paste()
{
    QMetaObject::invokeMethod(QApplication::focusWidget(), "paste");
}

void MainWindow::help()
{
    m_helpDialog->show();
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About SigmaSpectra"),
            QString(tr("SigmaSpectra was coded by Albert Kottke"
			   " and is released under the GNU General Public License (GPL). Feel free"
			  " to contact me regarding questions, comments, or requests for the source code."
				"<br><br>"
				"Albert Kottke <a href='mailto:albert@mail.utexas.edu'>albert@mail.utexas.edu</a>"
				"<br><br>"
				"Revision: %1"
               )).arg(REVISION));
}
        
void MainWindow::addRow()
{
    QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows();
    // Always add one layer at the end of the list
    if ( selectedRows.isEmpty() )
        m_tableView->model()->insertRows(m_tableView->model()->rowCount(), 1);
    else
        m_tableView->model()->insertRows( selectedRows.first().row(), selectedRows.size());
}    
    
void MainWindow::removeRow()
{
    QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows();

    m_tableView->model()->removeRows( selectedRows.first().row(), selectedRows.size());
	// Update the insert and remove buttons
    m_removeRowPushButton->setEnabled(false);
}
    
void MainWindow::selectPath()
{   
    QString dir =
            QFileDialog::getExistingDirectory(this, tr("Select Directory"),
                                              m_pathLineEdit->text().isEmpty() ?
                                              QDesktopServices::storageLocation(QDesktopServices::HomeLocation) : m_pathLineEdit->text(),
                                              QFileDialog::ShowDirsOnly
                                              | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        m_settings->setValue("mainWindow/path", dir);
        m_pathLineEdit->setText(QDir::toNativeSeparators(dir));
        m_motionLibrary->setMotionPath(dir);
    }
}

void MainWindow::compute()
{
    // Disable the group boxes
    m_targetGroupBox->setEnabled(false);
    m_periodGroupBox->setEnabled(false);
    m_libraryGroupBox->setEnabled(false);
    m_computePushButton->setEnabled(false);
    m_cancelPushButton->setEnabled(true);

    // Save the settings
    m_motionLibrary->save();

    // Reset the calculation information
    m_textEdit->clear();
    m_progressBar->setValue(0);
    m_etcLineEdit->clear();

    if (m_motionLibrary->compute()) {
        m_textEdit->append("<b>Success!<b>");

        // Display the suite dialog
        SuiteDialog dialog(m_motionLibrary, this);
        dialog.exec();
    }
    
    // Enable the group boxes
    m_targetGroupBox->setEnabled(true);
    m_periodGroupBox->setEnabled(true);
    m_libraryGroupBox->setEnabled(true);
    m_computePushButton->setEnabled(true);
    m_cancelPushButton->setEnabled(false);
}

void MainWindow::cellSelected()
{
    QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows();

    if (!selectedRows.isEmpty())
        m_removeRowPushButton->setEnabled(true);
    else
        m_removeRowPushButton->setEnabled(false);
} 

void MainWindow::updateSuiteSize(int suiteSize)
{
    m_seedSizeSpinBox->setMaximum(suiteSize);
    m_motionLibrary->setSuiteSize(suiteSize);
}

void MainWindow::updateMotionCount(int motionCount)
{
    m_suiteSizeSpinBox->setMaximum(motionCount - 1);
}

void MainWindow::flagMotions()
{
    if (m_motionLibrary->readMotions()) {
        FlagMotionsDialog dialog(m_motionLibrary->motions(),this);

        dialog.exec();

        int required = 0;
        int requested = 0;
        int disabled = 0;

        foreach (AbstractMotion * ab, m_motionLibrary->motions()) {
            switch (ab->flag()) {
                case AbstractMotion::Required:
                    ++required;
                    break;
                case AbstractMotion::Requested:
                    ++requested;
                    break;
                case AbstractMotion::Unmarked:
                    break;
                case AbstractMotion::Disabled:
                    ++disabled;
                    break;
            }
        }

        if (m_motionLibrary->suiteSize() < required)
            qCritical("Required number of motions is greater than suite size!");

        emit requiredCountChanged(required);
        emit requestedCountChanged(requested);
        emit disabledCountChanged(disabled);

        m_minRequestedCountSpinBox->setMaximum(qMin(requested,
                                                 m_motionLibrary->suiteSize()));
    }
}


void MainWindow::createActions()
{
    // Exit
    m_exitAction = new QAction(QIcon(":/images/process-stop.svg"), tr("&Exit"),this);
    m_exitAction->setShortcut(tr("Ctrl+q"));
    connect(m_exitAction, SIGNAL(triggered()), this, SLOT(close()));
    
    // Copy
    m_copyAction = new QAction(QIcon(":/images/edit-copy.svg"), tr("&Copy"),this);
    m_copyAction->setShortcut(tr("Ctrl+c"));
    connect(m_copyAction, SIGNAL(triggered()), this, SLOT(copy()));

    // Paste
    m_pasteAction = new QAction(QIcon(":/images/edit-paste.svg"), tr("&Paste"),this);
    m_pasteAction->setShortcut(tr("Ctrl+v"));
    connect(m_pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
    
    // Help
    m_helpAction = new QAction(QIcon(":/images/help-browser.svg"), tr("&Help"),this);
    m_helpAction->setShortcut(Qt::Key_F1);
    connect(m_helpAction, SIGNAL(triggered()), this, SLOT(help()));

    // About
    m_aboutAction = new QAction(tr("&About"),this);
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createPage()
{
    QGridLayout * layout;

    // Target spectrum
    layout = new QGridLayout;
    layout->setColumnStretch(0,1);

    m_dampingSpinBox = new QDoubleSpinBox;
    m_dampingSpinBox->setRange(0, 30);
    m_dampingSpinBox->setDecimals(1);
    m_dampingSpinBox->setSuffix(" %");
    m_dampingSpinBox->setSingleStep(1.);
    connect( m_dampingSpinBox, SIGNAL(valueChanged(double)), m_motionLibrary, SLOT(setDamping(double)));
    layout->addWidget( new QLabel(tr("Oscillator Damping:")), 0, 0, 1, 2 );
    layout->addWidget( m_dampingSpinBox, 0, 2 );
    
    m_tableView = new MyTableView;
    m_tableView->setModel(new InputTableModel(m_motionLibrary));
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    connect(m_tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(cellSelected()));
    layout->addWidget( m_tableView, 1, 0, 1, 3 );

    m_interpolateCheckBox = new QCheckBox(tr("Interpolate period"));
    connect(m_interpolateCheckBox, SIGNAL(toggled(bool)), m_motionLibrary, SLOT(setPeriodInterp(bool)));
    layout->addWidget( m_interpolateCheckBox, 2, 0 );

    m_addRowPushButton = new QPushButton(QIcon(":/images/list-add.svg"), tr("Add"));
    connect( m_addRowPushButton, SIGNAL(clicked()), this, SLOT(addRow()));
    layout->addWidget( m_addRowPushButton, 2,1);
    
    m_removeRowPushButton = new QPushButton(QIcon(":/images/list-remove.svg"), tr("Remove"));
    m_removeRowPushButton->setEnabled(false);
    connect( m_removeRowPushButton, SIGNAL(clicked()), this, SLOT(removeRow()));
    layout->addWidget( m_removeRowPushButton, 2,2);

    m_targetGroupBox = new QGroupBox(tr("Target Response Spectrum"));
    m_targetGroupBox->setLayout(layout);

    // Period interpolation
    QFormLayout *fLayout = new QFormLayout;

    m_periodSpacingComboBox = new QComboBox;
    m_periodSpacingComboBox->addItem(tr("Linear"));
    m_periodSpacingComboBox->addItem(tr("Logarithmic"));
    connect(m_periodSpacingComboBox, SIGNAL(currentIndexChanged(int)), m_motionLibrary, SLOT(setPeriodSpacing(int)));    
    fLayout->addRow(tr("Spacing:"), m_periodSpacingComboBox);

    m_periodMinSpinBox = new QDoubleSpinBox;
    m_periodMinSpinBox->setRange(0.01,10);
    m_periodMinSpinBox->setSingleStep(0.01);
    m_periodMinSpinBox->setSuffix(" s");
    connect(m_periodMinSpinBox, SIGNAL(valueChanged(double)), m_motionLibrary, SLOT(setPeriodMin(double)));    
    fLayout->addRow(tr("Minimum:"), m_periodMinSpinBox);
    
    m_periodMaxSpinBox = new QDoubleSpinBox;
    m_periodMaxSpinBox->setRange(0.01,10);
    m_periodMaxSpinBox->setSingleStep(0.01);
    m_periodMaxSpinBox->setSuffix(" s");
    connect(m_periodMaxSpinBox, SIGNAL(valueChanged(double)), m_motionLibrary, SLOT(setPeriodMax(double)));
    fLayout->addRow(tr("Maximum:"), m_periodMaxSpinBox);

    m_periodCountSpinBox = new QSpinBox;
    m_periodCountSpinBox->setRange(50,300);
    connect(m_periodCountSpinBox, SIGNAL(valueChanged(int)), m_motionLibrary, SLOT(setPeriodCount(int)));
    fLayout->addRow(tr("Number of Points:"), m_periodCountSpinBox);

    m_periodGroupBox = new QGroupBox(tr("Period Interpolation"));
    m_periodGroupBox->setEnabled(false);
    connect(m_interpolateCheckBox, SIGNAL(toggled(bool)), m_periodGroupBox, SLOT(setEnabled(bool)));
    m_periodGroupBox->setLayout(fLayout);
    
    // Library of motions
    QVBoxLayout * column = new QVBoxLayout;
    QHBoxLayout * row = new QHBoxLayout;

    QPushButton * pushButton = new QPushButton(QIcon(":images/folder.svg"), tr("Select Path"));
    connect( pushButton, SIGNAL(clicked()), this, SLOT(selectPath()));

    m_pathLineEdit = new QLineEdit;
    connect( m_pathLineEdit, SIGNAL(textEdited(QString)),
             m_motionLibrary, SLOT(setMotionPath(QString)));

    row->addWidget(pushButton);
    row->addWidget(m_pathLineEdit);
    column->addLayout(row);
    row = new QHBoxLayout;

    m_suiteSizeSpinBox = new QSpinBox;
    m_suiteSizeSpinBox->setRange( 2, 50 );
    connect( m_suiteSizeSpinBox, SIGNAL(valueChanged(int)),
             this, SLOT(updateSuiteSize(int)));

    row->addWidget(new QLabel(tr("Number of motions in suite:")));    
    row->addStretch();
    row->addWidget(m_suiteSizeSpinBox);
    column->addLayout(row);
    row = new QHBoxLayout;

    m_seedSizeSpinBox = new QSpinBox;
    m_seedSizeSpinBox->setRange( 1, 50 );
    connect( m_seedSizeSpinBox, SIGNAL(valueChanged(int)),
             m_motionLibrary, SLOT(setSeedSize(int)));

    row->addWidget(new QLabel(tr("Seed combination size:")));
    row->addStretch();
    row->addWidget(m_seedSizeSpinBox);
    column->addLayout(row);
    row = new QHBoxLayout;

    m_suiteCountSpinBox = new QSpinBox;
    m_suiteCountSpinBox->setRange( 2, 100 );
    connect( m_suiteCountSpinBox, SIGNAL(valueChanged(int)),
             m_motionLibrary, SLOT(setSuiteCount(int)));

    row->addWidget(new QLabel(tr("Suites to save:")));
    row->addStretch();
    row->addWidget(m_suiteCountSpinBox);
    column->addLayout(row);
    row = new QHBoxLayout;
    
    m_combinCheckBox = new QCheckBox(tr("Combine components"));
    connect(m_combinCheckBox, SIGNAL(toggled(bool)),
            m_motionLibrary, SLOT(setCombineComponents(bool)));
    
    m_stationCheckBox = new QCheckBox(tr("One component per recording station"));
    connect(m_combinCheckBox, SIGNAL(toggled(bool)),
            m_stationCheckBox, SLOT(setDisabled(bool)));
    connect(m_stationCheckBox, SIGNAL(toggled(bool)),
            m_motionLibrary, SLOT(setOneMotionPerStation(bool)));

    row->addWidget(m_combinCheckBox);
    row->addStretch();
    row->addWidget(m_stationCheckBox);
    column->addLayout(row);
    row = new QHBoxLayout;

    QFrame * hLine = new QFrame;
    hLine->setFrameShape(QFrame::HLine);
    column->addWidget(hLine);

    pushButton = new QPushButton(tr("Flag Motions..."));
    connect(pushButton, SIGNAL(clicked()), this, SLOT(flagMotions()));

    row->addWidget(pushButton);
    row->addStretch();

    QSpinBox * spinBox = new QSpinBox;
    spinBox->setRange(0, 9999);
    spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinBox->setReadOnly(true);
    connect(this, SIGNAL(requiredCountChanged(int)), spinBox, SLOT(setValue(int)));
    row->addWidget(new QLabel(tr("Required:")));
    row->addWidget(spinBox);

    spinBox = new QSpinBox;
    spinBox->setRange(0, 9999);
    spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinBox->setReadOnly(true);
    connect(this, SIGNAL(requestedCountChanged(int)), spinBox, SLOT(setValue(int)));
    row->addWidget(new QLabel(tr("Requested:")));
    row->addWidget(spinBox);

    spinBox = new QSpinBox;
    spinBox->setRange(0, 9999);
    spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinBox->setReadOnly(true);
    connect(this, SIGNAL(disabledCountChanged(int)), spinBox, SLOT(setValue(int)));
    row->addWidget(new QLabel(tr("Disabled:")));
    row->addWidget(spinBox);

    column->addLayout(row);
    row = new QHBoxLayout;

    m_minRequestedCountSpinBox = new QSpinBox;
    m_minRequestedCountSpinBox->setRange(0, 0);
    connect(m_minRequestedCountSpinBox, SIGNAL(valueChanged(int)),
            m_motionLibrary, SLOT(setMinRequestedCount(int)));
    row->addWidget(new QLabel(tr("Minimum Number of Requested Motions:")));
    row->addStretch();
    row->addWidget(m_minRequestedCountSpinBox);

    column->addLayout(row);
    row = new QHBoxLayout;

    hLine = new QFrame;
    hLine->setFrameShape(QFrame::HLine);
    column->addWidget(hLine);

    m_motionCountSpinBox = new QSpinBox;
    m_motionCountSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_motionCountSpinBox->setReadOnly(true);
    m_motionCountSpinBox->setRange(0, INT_MAX);
    connect( m_motionLibrary, SIGNAL(motionCountChanged(int)),
             m_motionCountSpinBox, SLOT(setValue(int)));

    row->addStretch();
    row->addWidget(new QLabel(tr("Motions found:")));
    row->addWidget(m_motionCountSpinBox);

    m_trialCountSpinBox = new QDoubleSpinBox;
    m_trialCountSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_trialCountSpinBox->setReadOnly(true);
    m_trialCountSpinBox->setDecimals(0);
    m_trialCountSpinBox->setRange(0, DBL_MAX);
    connect( m_motionLibrary, SIGNAL(trialCountChanged(double)),
             m_trialCountSpinBox, SLOT(setValue(double)));
    row->addWidget(new QLabel(tr("Number of trials:")));
    row->addWidget(m_trialCountSpinBox);

    column->addLayout(row);

    m_libraryGroupBox = new QGroupBox(tr("Library of Motions"));
    m_libraryGroupBox->setLayout(column);

    // Calculation
    layout = new QGridLayout;
    layout->setColumnStretch(0,1);

    m_textEdit = new QTextEdit;
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);
    connect( m_motionLibrary, SIGNAL(logText(QString)), m_textEdit, SLOT(append(QString)));
    layout->addWidget(m_textEdit, 0, 0, 1, 5);

    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 100);
    connect( m_motionLibrary, SIGNAL(percentChanged(int)), m_progressBar, SLOT(setValue(int)));
    layout->addWidget( m_progressBar, 1, 0 );

    m_etcLineEdit = new QLineEdit;
    m_etcLineEdit->setReadOnly(true);
    connect( m_motionLibrary, SIGNAL(timeChanged(QString)), m_etcLineEdit, SLOT(setText(QString)));
    layout->addWidget( new QLabel(tr("ETC:")), 1, 1);
    layout->addWidget( m_etcLineEdit, 1, 2 );

    m_cancelPushButton = new QPushButton(tr("Cancel"));
    m_cancelPushButton->setEnabled(false);
    connect( m_cancelPushButton, SIGNAL(clicked()), m_motionLibrary, SLOT(cancel()));
    layout->addWidget( m_cancelPushButton, 1, 3 );

    m_computePushButton = new QPushButton(tr("Compute"));
    connect( m_computePushButton, SIGNAL(clicked()), this, SLOT(compute()));
    layout->addWidget( m_computePushButton, 1, 4 );

    QGroupBox * calcGroupBox = new QGroupBox(tr("Calculation"));
    calcGroupBox->setLayout(layout);

    // set the layout of the main window
    layout = new QGridLayout;
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 2);
    layout->setRowStretch(1, 1);

    layout->addWidget( m_targetGroupBox, 0, 0, 2, 1 );
    layout->addWidget( m_periodGroupBox, 2, 0 );
    layout->addWidget( m_libraryGroupBox, 0, 1 );
    layout->addWidget( calcGroupBox, 1, 1, 2, 1 );

    QWidget * centralWidget = new QWidget;
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}

void MainWindow::createMenus()
{
    QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_exitAction);

    QMenu * editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(m_copyAction);
    editMenu->addAction(m_pasteAction);

    QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_helpAction);
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::createToolBar()
{
    QToolBar * toolbar = new QToolBar;

    toolbar->addAction(m_exitAction);
    toolbar->addSeparator();
    toolbar->addAction(m_copyAction);
    toolbar->addAction(m_pasteAction);
    toolbar->addSeparator();
    toolbar->addAction(m_aboutAction);

    // Add the toolbar to the mainwindow
    addToolBar(toolbar);
}
        
void MainWindow::loadValues()
{
    m_dampingSpinBox->setValue(m_motionLibrary->damping());
    m_interpolateCheckBox->setChecked( m_motionLibrary->periodInterp() );

    m_periodSpacingComboBox->setCurrentIndex((int)m_motionLibrary->periodSpacing());
    m_periodMinSpinBox->setValue(m_motionLibrary->periodMin());
    m_periodMaxSpinBox->setValue(m_motionLibrary->periodMax());
    m_periodCountSpinBox->setValue(m_motionLibrary->periodCount());
    
    m_pathLineEdit->setText(m_motionLibrary->motionPath());
    m_suiteSizeSpinBox->setValue(m_motionLibrary->suiteSize());
    m_seedSizeSpinBox->setValue(m_motionLibrary->seedSize());
    m_suiteCountSpinBox->setValue(m_motionLibrary->suiteCount());
    m_minRequestedCountSpinBox->setValue(m_motionLibrary->minRequestedCount());
    m_stationCheckBox->setChecked(m_motionLibrary->oneMotionPerStation());
    m_combinCheckBox->setChecked(m_motionLibrary->combineComponents());
    
    m_motionCountSpinBox->setValue(m_motionLibrary->motionCount());
    m_trialCountSpinBox->setValue(m_motionLibrary->trialCount());
}
