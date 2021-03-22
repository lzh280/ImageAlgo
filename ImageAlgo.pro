QT       += core gui widgets

QMAKE_CXXFLAGS += /utf-8

TARGET = ImageAlgo
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        LB_ImageProcess.cpp \
        graphicsview.cpp \
        main.cpp \
        ImageAlgo.cpp

HEADERS += \
        ImageAlgo.h \
    LB_ImageProcess.h \
        graphicsview.h \

FORMS += \
        ImageAlgo.ui

RESOURCES += \
    ImageAlgo.qrc
