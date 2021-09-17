QT       += core gui widgets

TARGET = ImageAlgo
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        3rd/dxflib/dl_dxf.cpp \
        3rd/dxflib/dl_writer_ascii.cpp \
        ImageProcessCommand.cpp \
        LB_Image/LB_BMPVectorization.cpp \
        LB_Image/LB_ElementDetection.cpp \
        LB_Image/LB_ImagePreProcess.cpp \
        LB_Image/LB_ImageViewer.cpp \
        main.cpp \
        ImageAlgo.cpp

HEADERS += \
    3rd/dxflib/dl_attributes.h \
    3rd/dxflib/dl_codes.h \
    3rd/dxflib/dl_creationadapter.h \
    3rd/dxflib/dl_creationinterface.h \
    3rd/dxflib/dl_dxf.h \
    3rd/dxflib/dl_entities.h \
    3rd/dxflib/dl_exception.h \
    3rd/dxflib/dl_extrusion.h \
    3rd/dxflib/dl_global.h \
    3rd/dxflib/dl_writer.h \
    3rd/dxflib/dl_writer_ascii.h \
        ImageAlgo.h \
    ImageProcessCommand.h \
    LB_Image/LB_BMPVectorization.h \
    LB_Image/LB_BaseUtil.h \
    LB_Image/LB_ContourElement.h \
    LB_Image/LB_ElementDetection.h \
    LB_Image/LB_ImagePreProcess.h \
    LB_Image/LB_ImageViewer.h \
    LB_ImageViewer.h \

FORMS += \
        ImageAlgo.ui

RESOURCES += \
    ImageAlgo.qrc
