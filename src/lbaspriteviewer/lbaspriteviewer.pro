######################################################################
# Automatically generated by qmake (3.0) Do. Aug. 30 22:08:23 2018
######################################################################

TEMPLATE     = app
TARGET       = lbaspriteviewer
INCLUDEPATH += .
QT          += widgets
CONFIG      += debug
QMAKE_CXXFLAGS += -std=c++11

include(../liblbadata/liblbadata.pro)
include(../liblbaworld/liblbaworld.pro)

# Input
SOURCES += main.cc \
    lbaspriteplayer.cc

HEADERS += \
    lbaspriteplayer.h

FORMS += \
    lbaspriteplayer.ui
