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

LIBS += -lgdal

SOURCES += main.cpp\
    mainwindow.cpp \
    datacanvas.cpp \
    drawlayer.cpp \
    projectionview.cpp \
    maplayer.cpp \
    ogrtools.cpp

HEADERS += mainwindow.h \
    datacanvas.h \
    drawlayer.h \
    projectionview.h \
    maplayer.h \
    ogrtools.h

FORMS += mainwindow.ui

OTHER_FILES += \
    main.qml \
    simple.frag \
    simple.vert \
    map.vert

RESOURCES += \
    resources.qrc
