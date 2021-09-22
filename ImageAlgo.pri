DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

SOURCES += \
        ImageProcessCommand.cpp \
        LB_Image/LB_BMPVectorization.cpp \
        LB_Image/LB_ElementDetection.cpp \
        LB_Image/LB_ImagePreProcess.cpp \
        LB_Image/LB_ImageViewer.cpp \
        main.cpp \
        ImageAlgo.cpp

HEADERS += \
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
