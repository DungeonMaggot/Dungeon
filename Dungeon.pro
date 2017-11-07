#-------------------------------------------------
#
# Project created by QtCreator 2011-03-04T17:34:20
#
#-------------------------------------------------

include(../Common/Common.pri)

TARGET = Dungeon
SOURCES += \
    dungeon.cpp

HEADERS  += planet.h \
    level0_map.h \
    floor_tile.h

FORMS += \
    dockwidget.ui

DISTFILES += \
    level0_map.txt \
    level0_entities.txt
