######################################################################
# Automatically generated by qmake (3.0) Sa. Sep. 22 12:20:29 2018
######################################################################

TEMPLATE    = app
TARGET      = lbwmodelviewer
INCLUDEPATH += .
QT          += core widgets
CONFIG      += debug c++11

include(../liblbadata/liblbadata.pro)
include(../liblbaworld/liblbaworld.pro)

LIBQTR3D_PATH = ../../../libqtr3d/
include(../../../libqtr3d/libqtr3d.pro)

# Input
SOURCES += main.cc \
    lbamodelviewer.cc

HEADERS += \
    lbamodelviewer.h

FORMS += \
    viewerform.ui