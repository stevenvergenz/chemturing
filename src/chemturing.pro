#-------------------------------------------------
#
# Project created by QtCreator 2012-02-24T05:21:33
#
#-------------------------------------------------

QT       += core sql

QT       -= gui

TARGET = chemturing
CONFIG   += console
CONFIG   -= app_bundle
#QMAKE_CXXFLAGS += -fpermissive

TEMPLATE = app


SOURCES += main.cpp \
	state.cpp \
	simulation.cpp \
	dispatcher.cpp \
	dbmanager.cpp

HEADERS += \
	state.h \
	simulation.h \
	dispatcher.h \
	dbmanager.h

LIBS += -lm
