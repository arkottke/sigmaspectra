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

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include "MotionLibrary.h"
#include "MyTableView.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTextEdit>

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QMainWindow *parent = 0);

    ~MainWindow();

public slots:

    void copy();

    void paste();

    void about();

    void help();

    void addRow();

    void removeRow();

    void selectPath();

    void compute();

protected slots:

    void cellSelected();

    void updateSuiteSize(int suiteSize);

    void updateMotionCount(int motionCount);

    void flagMotions();

signals:

    void requiredCountChanged(int count);

    void requestedCountChanged(int count);

    void disabledCountChanged(int count);

private:
    //! Create the actions
    void createActions();

    //! Create the tab widget and pages
    void createPage();

    //! Create the menus
    void createMenus();

    //! Create the toolbar
    void createToolBar();

    //! Load the values from the motion library
    void loadValues();

    QAction *m_exitAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;

    QAction *m_helpAction;
    QAction *m_aboutAction;

    QSettings *m_settings;

    // Input
    QGroupBox *m_targetGroupBox;
    MyTableView *m_tableView;
    QCheckBox *m_interpolateCheckBox;
    QDoubleSpinBox *m_dampingSpinBox;
    QPushButton *m_addRowPushButton;
    QPushButton *m_removeRowPushButton;

    QGroupBox *m_periodGroupBox;
    QComboBox *m_periodSpacingComboBox;
    QDoubleSpinBox *m_periodMinSpinBox;
    QDoubleSpinBox *m_periodMaxSpinBox;
    QSpinBox *m_periodCountSpinBox;

    QGroupBox *m_libraryGroupBox;
    QLineEdit *m_pathLineEdit;
    QSpinBox *m_suiteSizeSpinBox;
    QSpinBox *m_seedSizeSpinBox;
    QSpinBox *m_suiteCountSpinBox;
    QCheckBox *m_stationCheckBox;
    QCheckBox *m_combinCheckBox;
    QSpinBox *m_minRequestedCountSpinBox;
    QSpinBox *m_motionCountSpinBox;
    QDoubleSpinBox *m_trialCountSpinBox;

    QTextEdit *m_textEdit;
    QProgressBar *m_progressBar;
    QLineEdit *m_etcLineEdit;
    QPushButton *m_computePushButton;
    QPushButton *m_cancelPushButton;

    MotionLibrary *m_motionLibrary;
};

#endif
