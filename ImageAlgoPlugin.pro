QT       += core gui widgets

TEMPLATE = lib
CONFIG      += plugin
TARGET      = $${RLIBNAME}ImageAlgo
include(../../shared.pri)

HEADERS     = ImageAlgoPlugin.h
SOURCES     = ImageAlgoPlugin.cpp
DESTDIR     = $$PWD/../../plugins
LIBS += -l$${RLIBNAME}core -l$${RLIBNAME}gui -l$${RLIBNAME}ecmaapi -l$${RLIBNAME}entity -l$${RLIBNAME}operations

include($$PWD/ImageAlgo.pri)
