#-------------------------------------------------
#
# Project created by QtCreator 2014-12-21T01:32:06
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = CpuProto
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ram.cpp \
    prom.cpp \
    mproc8080.cpp

HEADERS += \
    ram.h \
    prom.h \
    mproc8080.h
