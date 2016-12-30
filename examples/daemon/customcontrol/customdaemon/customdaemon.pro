!include(../../examples.pri)  {
    error("Couldn't find the examples.pri file!")
} 

TARGET = customdaemon

SOURCES += main.cpp \
    customdaemon.cpp

HEADERS += \
    customdaemon.h
