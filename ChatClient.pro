TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -pthread

SOURCES += main.cpp \
    logger.cpp \
    protocol.cpp \
    client.cpp

HEADERS += \
    common.h \
    logger.h \
    protocol.h \
    serialization.h \
    client.h \
    queue.h \
    inputargreader.h
