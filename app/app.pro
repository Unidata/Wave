#-------------------------------------------------
#
# Project created by QtCreator 2014-04-21T10:18:31
#
#-------------------------------------------------

CONFIG += c++11

#QMAKE_CXXFLAGS += -std=c++1y

QT += core gui widgets quick quickwidgets

TARGET = wave
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    datacanvas.cpp

HEADERS += mainwindow.h \
    datacanvas.h

FORMS += mainwindow.ui

OTHER_FILES += \
    main.qml \
    simple.frag \
    simple.vert

RESOURCES += \
    resources.qrc
