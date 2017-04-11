TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    config.cpp \
    radio.cpp \
    inih/INIReader.cpp \
    inih/ini.c \
    k3commands.cpp \
    skanticommands.cpp

GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags --long)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

QMAKE_CXXFLAGS += -Ofast -mfpu=vfp -mfloat-abi=hard -std=c++11

HEADERS += \
    config.h \
    inih/INIReader.h \
    inih/ini.h \
    radio.h
