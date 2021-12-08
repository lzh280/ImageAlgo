QT += core gui widgets svg

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
        LB_Image/LB_VectorThread.cpp \
        LB_QtTool/ImageProcessCommand.cpp \
        LB_QtTool/ImageVectorCommand.cpp \
        LB_QtTool/LB_DebugHandle.cpp \
        LB_QtTool/QStateProgressBar.cpp \
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
        LB_Image/LB_VectorThread.h \
        LB_QtTool/ImageProcessCommand.h \
        LB_QtTool/ImageVectorCommand.h \
        LB_QtTool/LB_DebugHandle.h \
        LB_QtTool/QStateProgressBar.h \
        MainWindow.h

FORMS += \
    MainWindow.ui

RESOURCES += \
    ImageVectorization.qrc

TRANSLATIONS = ImageVectorization_zh_CN.ts

INCLUDEPATH += $$PWD/../dxflib

mingw {
    CONFIG(debug, debug|release) {
       LIBS += $$PWD/../bind/libdxflib.a
       DESTDIR = $$PWD/../bind
    } else {
        LIBS += $$PWD/../bin/libdxflib.a
        DESTDIR = $$PWD/../bin
    }
} else {
    CONFIG(debug, debug|release) {
        LIBS += $$PWD/../bind/dxflib.lib
        DESTDIR = $$PWD/../bind
    } else {
        LIBS += $$PWD/../bin/dxflib.lib
        DESTDIR = $$PWD/../bin
    }
}

contains(DEFINES,USE_OPENCV) {
    SOURCES += LB_Image/LB_VectorCVHandle.cpp
    HEADERS += LB_Image/LB_VectorCVHandle.h

    LIBS += D:/OpenCV/bin/libopencv_world454.dll
    INCLUDEPATH += D:\OpenCV\include

    message('USEING OPENCV')
} else {
    message('WITHOUT OPENCV, USE CUSTOM ALGORITHM')
}
