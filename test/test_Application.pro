# -------------------------------------------------
# Project created by QtCreator 2015-03-16T11:11:04
# -------------------------------------------------
QT -= core \
    gui
TARGET = test_Application
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
INCLUDEPATH += ../STFramework
INCLUDEPATH += ../STFramework/include

SOURCES += ./main.cpp \
    ../STFramework/core/STCoreApplication.cpp \
    ../STFramework/core/STObject.cpp \
    ../STFramework/event/STEvent.cpp \
    ../STFramework/thread/STThread.cpp \
    ../STFramework/base/STWaiter.cpp \
    ../STFramework/base/STGuard.cpp \
    ../STFramework/base/STMutex.cpp \
    ../STFramework/tools/STDataItem.cpp \
    ../STFramework/core/STEventDispatcher.cpp \
    ../STFramework/net/STServer.cpp \
    ../STFramework/net/STClient.cpp \
    ../STFramework/net/STNetIdentify.cpp

HEADERS += ../STFramework/include/core/STCoreApplication.h \
    ../STFramework/include/core/STObject.h \
    ../STFramework/include/event/STEvent.h \
    ../STFramework/include/thread/STThread.h \
    ../STFramework/include/base/STCommonDefine.h \
    ../STFramework/include/base/STWaiter.h \
    ../STFramework/include/base/STPtr.h \
    ../STFramework/include/base/STGuard.h \
    ../STFramework/include/base/STMutex.h \
    ../STFramework/include/tools/STDataItem.h \
    ../STFramework/include/core/STEventDispatcher.h \
    ../STFramework/include/core/STIEventDispatcher.h \
    ../STFramework/include/net/STServer.h \
    ../STFramework/include/net/STClient.h \
    ../STFramework/include/net/STNetEvent.h \
    ../STFramework/include/net/STNetIdentify.h
