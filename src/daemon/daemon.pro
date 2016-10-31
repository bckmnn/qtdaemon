load(qt_build_config)

TARGET = QtDaemon
MODULE = daemon

QT = core
unix:!macx  {
    QT += dbus
}

CONFIG(debug, debug|release):*g++ {
    QMAKE_CXXFLAGS += --pedantic-errors
}

DEFINES += QT_NO_FOREACH QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

include($$PWD/daemon-lib.pri)

load(qt_module)
