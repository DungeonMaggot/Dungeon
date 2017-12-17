#-------------------------------------------------
#
# Project created by QtCreator 2011-03-04T17:34:20
#
#-------------------------------------------------

include(../Common/Common.pri)

TARGET = Dungeon
SOURCES += \
    dungeon.cpp

HEADERS  += \
    level0_map.h \
    input.h \
    dungeon.h \
    utility.h

FORMS += \
    dockwidget.ui

DISTFILES += \
    level0_map.txt \
    level0_entities.txt \
    shaders/texture.frag \
    shaders/texture.vert

RESOURCES += \
    shaders.qrc
