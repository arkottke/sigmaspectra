########################################################################
# SigmaSpectra
# Copyright (C) 2011-16   Albert R. Kottke
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GPL License, Version 3.0
########################################################################

TEMPLATE = app
TARGET = sigmaspectra
QT += gui widgets xml core

# Version information
VER_MAJ = 0
VER_MIN = 5
VER_PAT = 5
GIT_VER = $$system(git rev-parse --short HEAD)
DEFINES += "VERSION=\\\"$${VER_MAJ}.$${VER_MIN}.$${VER_PAT}-$${GIT_VER}\\\""

# Load configuration from sigmaSpectraConfig.pri if it exists
include(sigmaSpectraConfig.pri)

# Required libraries for linking.
LIBS += $$(LIBS) -lgsl -lgslcblas

win32:debug {
    LIBS += -lqwtd
} else {
    LIBS += -lqwt
}

# Build type. For most cases this should be release, however during
# development of the software using the debug configuration can be
# beneficial.
#
# This can be specified at build time with, such as::
#   $> make release
CONFIG += debug_and_release

# Add warning messages and support for c++14
CONFIG += c++14 warn_on

# Configuration for release and debug versions
CONFIG(debug, debug|release) {
    # Enable console for debug versions
    CONFIG += console
    # Flag based on if the program is compiled in debug mode.
    DEFINES += DEBUG
    # Build to debug
    DESTDIR = debug
} else {
    # Build to release
    DESTDIR = release
}

# Add the icon for windows binaries
win32 {
    RC_FILE = sigmaSpectra.rc
}


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
