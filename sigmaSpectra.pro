# All settings should be modified from the sigmaSpectraConfig.pri file
include(sigmaspectraconfig.pri)

# Grab the revision number using svnversion and clean it up.
DEFINES += REVISION=$$system(python getSvnVersion.py)

# Flag based on if the program is compiled in debug mode. 
CONFIG(debug, debug|release) {
   DEFINES += DEBUG
}

# Directories for building
CONFIG(debug, debug|release) {
   DESTDIR = debug
} else {
   DESTDIR = release
}

TEMPLATE = app
TARGET = sigmaspectra
QT += xml

HEADERS += src/AbstractMotion.h \
    src/AxisOptionsGroupBox.h \
    src/ConfigurePlotDialog.h \
    src/ExportDialog.h \
    src/InputTableModel.h \
    src/MainWindow.h \
    src/Motion.h \
    src/MotionPair.h \
    src/MotionLibrary.h \
    src/MotionSuite.h \
    src/MyTableView.h \
    src/SuiteDialog.h \
    src/FlagMotionsDialog.h \
    src/StringListDelegate.h

SOURCES += src/AbstractMotion.cpp \
    src/AxisOptionsGroupBox.cpp \
    src/ConfigurePlotDialog.cpp \
    src/ExportDialog.cpp \
    src/InputTableModel.cpp \
    src/main.cpp \
    src/MainWindow.cpp \
    src/Motion.cpp \
    src/MotionPair.cpp \
    src/MotionLibrary.cpp \
    src/MotionSuite.cpp \
    src/MyTableView.cpp \
    src/SuiteDialog.cpp \
    src/FlagMotionsDialog.cpp \
    src/StringListDelegate.cpp

RESOURCES += resources/sigmaSpectra.qrc
