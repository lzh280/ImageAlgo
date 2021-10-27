TARGET = dxflib
TEMPLATE = lib
CONFIG -= qt
DEFINES += DXFLIB_DLL DXFLIB_LIBRARY

HEADERS = \
    dl_attributes.h \
    dl_codes.h \
    dl_creationadapter.h \
    dl_creationinterface.h \
    dl_dxf.h \
    dl_entities.h \
    dl_exception.h \
    dl_extrusion.h \
    dl_global.h \
    dl_writer.h \
    dl_writer_ascii.h

SOURCES = \
    dl_dxf.cpp \
    dl_writer_ascii.cpp

CONFIG(debug, debug|release){
    DESTDIR = $$PWD/../bind
}else {
    DESTDIR = $$PWD/../bin
}
