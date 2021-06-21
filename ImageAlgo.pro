QT       += core gui widgets

TARGET = ImageAlgo
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        3rd/QxPotrace/potrace/bbox.c \
        3rd/QxPotrace/potrace/curve.c \
        3rd/QxPotrace/potrace/decompose.c \
        3rd/QxPotrace/potrace/potracelib.c \
        3rd/QxPotrace/potrace/progress_bar.c \
        3rd/QxPotrace/potrace/trace.c \
        3rd/QxPotrace/potrace/trans.c \
        3rd/QxPotrace/qxpotrace.cpp \
        3rd/dxflib/dl_dxf.cpp \
        3rd/dxflib/dl_writer_ascii.cpp \
        ImageProcessCommand.cpp \
        LB_Image/LB_ImageProcess.cpp \
        LB_Image/LB_ImageViewer.cpp \
        main.cpp \
        ImageAlgo.cpp

HEADERS += \
    3rd/QxPotrace/QxPotrace \
    3rd/QxPotrace/bezier.h \
    3rd/QxPotrace/potrace/auxiliary.h \
    3rd/QxPotrace/potrace/bbox.h \
    3rd/QxPotrace/potrace/bitmap.h \
    3rd/QxPotrace/potrace/curve.h \
    3rd/QxPotrace/potrace/decompose.h \
    3rd/QxPotrace/potrace/lists.h \
    3rd/QxPotrace/potrace/potracelib.h \
    3rd/QxPotrace/potrace/progress.h \
    3rd/QxPotrace/potrace/progress_bar.h \
    3rd/QxPotrace/potrace/trace.h \
    3rd/QxPotrace/potrace/trans.h \
    3rd/QxPotrace/qxpotrace.h \
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
    LB_Image/LB_ImageProcess.h \
    LB_Image/LB_ImageViewer.h \
    LB_ImageViewer.h \

FORMS += \
        ImageAlgo.ui

RESOURCES += \
    ImageAlgo.qrc

INCLUDEPATH += $$PWD/3rd/dxflib \
                $$PWD/3rd/QxPotrace
