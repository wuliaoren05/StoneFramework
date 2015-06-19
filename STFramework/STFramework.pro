QT       -= core gui

TARGET = STFramework
TEMPLATE = lib
DEFINES += STONEFRAMEWORK_LIBRARY


INCLUDEPATH += include

SOURCES += \
    event/STEvent.cpp \
    thread/STThread.cpp \
    base/STWaiter.cpp \
    base/STGuard.cpp \
    base/STMutex.cpp \
    base/STBuffer.cpp \
    core/STEventDispatcher.cpp \
    core/STCoreApplication.cpp \
    core/STObject.cpp \
    net/STServer.cpp \
    net/STClient.cpp \
    net/STNetIdentify.cpp \
    net/STFdListeenerBase.cpp \
    net/SocketFdReader.cpp \
    tools/STDataItem.cpp \
    tools/STStringTool.cpp \
    tools/STIOTools.cpp \
    tools/STBase64.cpp \
    netext/STServerExt.cpp \
    tools/STRandom.cpp \
    log/STLog.cpp



HEADERS += \
    include/core/STCoreApplication.h \
    include/core/STObject.h \
    include/core/STEventDispatcher.h \
    include/core/STIEventDispatcher.h \
    include/event/STEvent.h \
    include/thread/STThread.h \
    include/base/STCommonDefine.h \
    include/base/STWaiter.h \
    include/base/STPtr.h \
    include/base/STGuard.h \
    include/base/STMutex.h \
    include/base/STBuffer.h \
    include/net/STServer.h \
    include/net/STClient.h \
    include/net/STNetEvent.h \
    include/net/STNetIdentify.h \
    include/net/STFdListeenerBase.h \
    include/net/STNetDefine.h \
    include/tools/STDataItem.h \
    include/tools/STStringTool.h \
    include/tools/STIOTools.h \
    include/tools/STBase64.h \
    net/SocketFdReader.h \
    include/netext/STServerExt.h \
    include/tools/STRandom.h \
    include/log/STLog.h






unix {
    target.path = /usr/lib
    INSTALLS += target
}

