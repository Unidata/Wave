# Necessary for getting it to build against latest OpenGL headers
macx: QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
macx: QMAKE_MAC_SDK = macosx10.9

CONFIG += c++11

#QMAKE_CXXFLAGS += -std=c++1y

QT += core gui widgets quick quickwidgets

TARGET = wave
TEMPLATE = app

PKGCONFIG += gdal

# Find GDAL libs on system
LIBS += $$system(gdal-config --libs)

# Find location of GDAL includes. Need to strip off leading -I to use
# in INCCLUDEPATH, which we do so that it's also available for Creator
# code model
GDAL_INCLUDES = $$system(gdal-config --cflags)
GDAL_INCLUDES ~= s/^-I//
INCLUDEPATH += $$GDAL_INCLUDES

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
