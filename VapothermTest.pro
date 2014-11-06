#-------------------------------------------------
#
# Project created by QtCreator 2014-05-27T21:25:32
#
#-------------------------------------------------

QT       += core gui
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VapothermTest
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    TestScript.cpp \
    Abort.cpp \
    Command.cpp

HEADERS  += mainwindow.h \
    TestScript.h \
    Abort.h \
    Command.h

FORMS    += mainwindow.ui
