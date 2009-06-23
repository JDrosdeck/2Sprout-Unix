#-------------------------------------------------
#
# Project created by QtCreator 2009-05-27T16:45:37
#
#-------------------------------------------------

TARGET = 2sprout
TEMPLATE = app
RESOURCES += files.qrc

SOURCES += main.cpp\
        sprout.cpp

HEADERS  += sprout.h

FORMS    += sprout.ui

mystaticconfig{
    QMAKE_LIBS_QT =
LIBS += /usr/local/TrollTech/Qt-4.6.0/lib/libQtCore.a
LIBS += /usr/local/TrollTech/Qt-4.6.0/lib/libQtGui.a -lz -framework Carbon
}
