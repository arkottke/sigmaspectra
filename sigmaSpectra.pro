TEMPLATE = app
CONFIG += warn_on \
    debug_and_release
QT += xml
TARGET = 
DEPENDPATH += . \
    resources \
    src
INCLUDEPATH += .
win32 { 
    DEFINES += REVISION=$$system(cmd.exe /C "SubWCRev.exe . | perl.exe -ne\"if (/^Last/){s/\D+//; print;}\"")
    LIBS += -lm \
        -lfftw3-3 \
        -L"C:\devel\fftw-3.2.2" \
        -lgsl \
        -lgslcblas \
        -L"C:\devel\GnuWin32\lib"
    INCLUDEPATH += . \
        "C:\devel\fftw-3.2.2" \
        "C:\devel\qwt-5.2\src" \
        "C:\devel\GnuWin32\include"
    RC_FILE = sigmaSpectra.rc
    CONFIG( debug, debug|release ) {
        # debug
       LIBS += -lqwtd5 \
           -L"C:\devel\qwt-5.2\lib"
    } else{
        # release
       LIBS += -lqwt5 \
          -L"C:\devel\qwt-5.2\lib"
    }
}
unix { 
    DEFINES += REVISION=$$system("svnversion . | perl -pne's/(?:\d+:)?(\d+)(?:[MS]+)?$/\1/'")
    LIBS += -lm \
        -lfftw3 \
        -lgsl \
        -lqwt \
        -lgslcblas
    INCLUDEPATH += . \
        "/usr/include/" \
        "/usr/include/qwt"
}

# Input
HEADERS += src/AbstractMotion.h \
    src/AxisOptionsGroupBox.h \
    src/ConfigurePlotDialog.h \
    src/ExportDialog.h \
    src/HelpDialog.h \
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
    src/HelpDialog.cpp \
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
