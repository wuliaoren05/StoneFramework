QT       -= core gui

TARGET = STFramework
TEMPLATE = lib
DEFINES += STONEFRAMEWORK_LIBRARY


INCLUDEPATH += include

SOURCES += \
    core/STCoreApplication.cpp \
    core/STObject.cpp \
    event/STEvent.cpp \
    thread/STThread.cpp \
    base/STWaiter.cpp \
    base/STGuard.cpp \
    base/STMutex.cpp \
    tools/STDataItem.cpp \
    core/STEventDispatcher.cpp \
    net/STServer.cpp \
    net/STClient.cpp \
    net/STNetIdentify.cpp \
    net/STFdListeenerBase.cpp \
    net/SocketFdReader.cpp \
    tools/STStringTool.cpp \
    tools/STIOTools.cpp


HEADERS += \
    include/core/STCoreApplication.h \
    include/core/STObject.h \
    include/event/STEvent.h \
    include/thread/STThread.h \
    include/base/STCommonDefine.h \
    include/base/STWaiter.h \
    include/base/STPtr.h \
    include/base/STGuard.h \
    include/base/STMutex.h \
    include/tools/STDataItem.h \
    include/core/STEventDispatcher.h \
    include/core/STIEventDispatcher.h \
    include/net/STServer.h \
    include/net/STClient.h \
    include/net/STNetEvent.h \
    include/net/STNetIdentify.h \
    include/net/STFdListeenerBase.h \
    include/net/STNetDefine.h \
    include/tools/STStringTool.h \
    include/tools/STIOTools.h \
    net/SocketFdReader.h




unix {
    target.path = /usr/lib
    INSTALLS += target
}

