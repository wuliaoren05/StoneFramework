QT      -= core gui

TARGET  = clientExt
CONFIG  -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../STFramework
INCLUDEPATH += ../../STFramework/include

LIBS += -L$$OUT_PWD/../../STFramework/ -lSTFramework





SOURCES += main.cpp
