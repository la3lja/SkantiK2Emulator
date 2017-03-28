TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    config.cpp \
    radio.cpp \
    inih/INIReader.cpp \
    inih/ini.c \
    k3commands.cpp

#QMAKE_CFLAGS += -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -std=c++11
QMAKE_CXXFLAGS += -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -std=c++11

HEADERS += \
    config.h \
    inih/INIReader.h \
    inih/ini.h \
    radio.h
