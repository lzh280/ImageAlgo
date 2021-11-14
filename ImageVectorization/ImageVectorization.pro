QT += core gui widgets

TEMPLATE = app

TARGET = ImageVectorization

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

RC_FILE += $$PWD/icons/ImageVectorization.rc

SOURCES += \
        LB_Graphics/LB_GraphicsItem.cpp \
        LB_Graphics/LB_PointItem.cpp \
        LB_Image/LB_BMPVectorization.cpp \
        LB_Image/LB_ElementDetection.cpp \
        LB_Image/LB_ImagePreProcess.cpp \
        LB_Image/LB_ImageViewer.cpp \
        LB_QtTool/ImageProcessCommand.cpp \
        LB_QtTool/ImageVectorCommand.cpp \
        LB_QtTool/LB_DebugHandle.cpp \
        main.cpp \
        MainWindow.cpp \

HEADERS += \
        LB_Graphics/LB_GraphicsItem.h \
        LB_Graphics/LB_PointItem.h \
        LB_Image/LB_BMPVectorization.h \
        LB_Image/LB_BaseUtil.h \
        LB_Image/LB_ContourElement.h \
        LB_Image/LB_ElementDetection.h \
        LB_Image/LB_ImagePreProcess.h \
        LB_Image/LB_ImageViewer.h \
        LB_QtTool/ImageProcessCommand.h \
        LB_QtTool/ImageVectorCommand.h \
        LB_QtTool/LB_DebugHandle.h \
        MainWindow.h

RESOURCES += \
    ImageVectorization.qrc

TRANSLATIONS = ImageVectorization_zh_CN.ts

INCLUDEPATH += $$PWD/../SARibbonBar
INCLUDEPATH += $$PWD/../dxflib

CONFIG(debug, debug|release) {
    LIBS += $$PWD/../bind/dxflib.dll
    DESTDIR = $$PWD/../bind
} else {
    LIBS += $$PWD/../bin/dxflib.dll
    DESTDIR = $$PWD/../bin
}

FORMS += \
    MainWindow.ui
