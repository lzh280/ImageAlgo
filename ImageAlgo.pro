QT       += core gui widgets

TARGET = ImageAlgo
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        ImageProcessCommand.cpp \
        LB_ImageProcess.cpp \
        LB_ImageViewer.cpp \
        main.cpp \
        ImageAlgo.cpp

HEADERS += \
        ImageAlgo.h \
    ImageProcessCommand.h \
    LB_ImageProcess.h \
    LB_ImageViewer.h \

FORMS += \
        ImageAlgo.ui

RESOURCES += \
    ImageAlgo.qrc
