# Necessary for getting it to build against latest OpenGL headers
macx: QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
macx: QMAKE_MAC_SDK = macosx10.9

CONFIG += c++11

#QMAKE_CXXFLAGS += -std=c++1y

QT += core gui widgets quick quickwidgets opengl xml

TARGET = wave
TEMPLATE = app

# Find GDAL libs on system
LIBS += $$system(gdal-config --libs)
LIBS += $$system(ncxx4-config --libs)

# Remove default directories, since having them here screws up
# linking against OpenGL on linux
linux: LIBS -= -L/usr/lib64 -L/usr/lib

# Find location of GDAL includes. Need to strip off leading -I to use
# in INCCLUDEPATH, which we do so that it's also available for Creator
# code model
GDAL_INCLUDES = $$system(gdal-config --cflags)
GDAL_INCLUDES ~= s/^-I//
INCLUDEPATH += $$GDAL_INCLUDES
INCLUDEPATH += $$system(ncxx4-config --includedir)

SOURCES += main.cpp\
    mainwindow.cpp \
    datacanvas.cpp \
    drawlayer.cpp \
    projectionview.cpp \
    maplayer.cpp \
    ogrtools.cpp \
    rasterimagelayer.cpp \
    radarlayer.cpp \
    pointslayer.cpp \
    hurricanelayer.cpp

HEADERS += mainwindow.h \
    datacanvas.h \
    drawlayer.h \
    projectionview.h \
    maplayer.h \
    ogrtools.h \
    rasterimagelayer.h \
    radarlayer.h \
    pointslayer.h \
    hurricanelayer.h

FORMS += mainwindow.ui

OTHER_FILES += \
    main.qml \
    label.qml \
    simple.frag \
    simple.vert \
    map.vert \
    texture.frag \
    texture.vert

RESOURCES += \
    resources.qrc
