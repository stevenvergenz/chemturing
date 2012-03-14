#-------------------------------------------------
#
# Project created by QtCreator 2012-02-24T05:21:33
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = chemturing
CONFIG   += console
CONFIG   -= app_bundle
QMAKE_CXXFLAGS += -fpermissive

TEMPLATE = app


SOURCES += src/main.cpp \
	src/state.cpp \
	src/simulation.cpp

HEADERS += src/state.h \
	src/simulation.h

