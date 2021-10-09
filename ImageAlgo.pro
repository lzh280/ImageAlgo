QT       += core gui widgets

TEMPLATE = app

TARGET      = ImageAlgo

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

SOURCES += \    
        ImageProcessCommand.cpp \
        LB_Graphics/LB_GraphicsItem.cpp \
        LB_Graphics/LB_PointItem.cpp \
        LB_Image/LB_BMPVectorization.cpp \
        LB_Image/LB_ElementDetection.cpp \
        LB_Image/LB_ImagePreProcess.cpp \
        LB_Image/LB_ImageViewer.cpp \
        main.cpp \
        MainWindow.cpp \

HEADERS += \
        ImageProcessCommand.h \
        LB_Graphics/LB_GraphicsItem.h \
        LB_Graphics/LB_PointItem.h \
        LB_Image/LB_BMPVectorization.h \
        LB_Image/LB_BaseUtil.h \
        LB_Image/LB_ContourElement.h \
        LB_Image/LB_ElementDetection.h \
        LB_Image/LB_ImagePreProcess.h \
        LB_Image/LB_ImageViewer.h \
        LB_ImageViewer.h \
        MainWindow.h

FORMS += \
        MainWindow.ui

RESOURCES += \
    ImageAlgo.qrc

TRANSLATIONS = ImageAlgo_zh_CN.ts

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/bind
} else {
    DESTDIR = $$PWD/bin
}
