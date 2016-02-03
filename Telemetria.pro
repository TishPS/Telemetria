#-------------------------------------------------
#
# Project created by QtCreator 2015-12-14T21:03:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport serialport

TARGET = Telemetria
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    parser.cpp \
    qcustomplot.cpp \
    settingsdialog.cpp

HEADERS  += mainwindow.h \
    parser.h \
    qcustomplot.h \
    settingsdialog.h

FORMS    += mainwindow.ui \
    settingsdialog.ui
