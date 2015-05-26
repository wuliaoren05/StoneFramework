# -------------------------------------------------
# Project created by QtCreator 2015-03-16T11:11:04
# -------------------------------------------------
QT -= core gui
TARGET = test
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
INCLUDEPATH += ../STFramework
INCLUDEPATH += ../STFramework/include

LIBS += -L$$OUT_PWD/../STFramework/ -lSTFramework

SOURCES += ./main.cpp
