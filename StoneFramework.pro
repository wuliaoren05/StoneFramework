TEMPLATE = subdirs


SUBDIRS = \
        STFramework \
        test \
        demo


unix:!macx: LIBS += -L$$OUT_PWD/STFramework/ -lSTFramework

INCLUDEPATH += $$PWD/STFramework
DEPENDPATH += $$PWD/STFramework
