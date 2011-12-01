TEMPLATE = app
CONFIG += warn_on \
    debug_and_release
QT += xml

# DEPENDPATH += . \
#     resources \
#     src

# Grab the revision number using svnversion. This is later cleaned up using a regular expression
DEFINES += REVISION=$$system(python getSvnVersion.py)

unix {
    DEFINES += GSL_LIB=$$system("env | grep GSL_LIB")
    DEFINES += GSL_INCLUDE=$$system("env | grep GSL_INCLUDE")
    LIBS += -lm \
        -lfftw3 \
        -lgsl \
        -lgslcblas \
        -L\${GSL_LIB} \
        -lqwt-qt4
    INCLUDEPATH += . \
        "/usr/include/qwt-qt4" \
        \${GSL_INCLUDE}
    target.path = bin
    INSTALLS = target
}

win32 { 
    LIBS += -lm \
        -lfftw3-3 \
        -L"C:/devel/fftw-3.2.2" \
        -lgsl \
        -lgslcblas \
        -L"C:/devel/GnuWin32/bin"
    INCLUDEPATH += . \
        "C:/devel/fftw-3.2.2" \
        "C:/devel/qwt-6.0/src" \
        "C:/devel/GnuWin32/include"
    RC_FILE = sigmaSpectra.rc
    CONFIG(debug, debug|release ) {
        LIBS += -lqwtd \
            -L"C:/devel/qwt-6.0/lib"
    } else {
        LIBS += -lqwt \
            -L"C:/devel/qwt-6.0/lib"
    }
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
